
#include "Rasterizer.h"

#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
#include <GL/glew.h>
#endif

#include <vector>

#include <skia/core/SkPath.h>
#include <skia/core/SkPathEffect.h>
#include <skia/core/SkColorSpace.h>
#include <skia/core/SkSurface.h>
#include <skia/core/SkCanvas.h>
#include <skia/utils/SkParsePath.h>
#include <skia/pathops/SkPathOps.h>
#include <skia/effects/SkDashPathEffect.h>

#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
#ifndef SK_GL
    #error Skia OpenGL support required
#endif
#include <skia/gpu/GrDirectContext.h>
#include <skia/gpu/GrBackendSurface.h>
#include <skia/gpu/gl/GrGLInterface.h>
#endif

#include <ode/utils.h>
#include <ode/geometry/RectangleMargin.h>

namespace ode {

static constexpr double defaultMiterLimit = 10;
static constexpr octopus::VectorStroke::LineJoin defaultLineJoin = octopus::VectorStroke::LineJoin::MITER;
static constexpr octopus::VectorStroke::LineCap defaultLineCap = octopus::VectorStroke::LineCap::BUTT;

class Rasterizer::Shape {
public:
    octopus::Shape octopusShape;
    SkPath bodyPath;
    std::vector<SkPath> strokePaths;
};

class Rasterizer::Internal {

#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
    sk_sp<GrDirectContext> graphicsContext;
#endif

public:
#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
    GrDirectContext * getGraphicsContext();
#endif
    bool rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, SkSurface *surface);

};

Rasterizer::ShapePtr::ShapePtr(std::nullptr_t) { }
Rasterizer::ShapePtr::ShapePtr(Shape *ptr) : std::unique_ptr<Shape>(ptr) { }
Rasterizer::ShapePtr::ShapePtr(ShapePtr &&orig) = default;
Rasterizer::ShapePtr::~ShapePtr() = default;
Rasterizer::ShapePtr &Rasterizer::ShapePtr::operator=(ShapePtr &&orig) = default;

Rasterizer::Rasterizer() : data(new Internal) { }

Rasterizer::Rasterizer(Rasterizer &&orig) = default;
Rasterizer::~Rasterizer() = default;
Rasterizer &Rasterizer::operator=(Rasterizer &&orig) = default;

static SkMatrix makeMatrix(const Matrix3x2d &transformation) {
    SkScalar affine[6] = { SkScalar(transformation[0][0]), SkScalar(transformation[0][1]), SkScalar(transformation[1][0]), SkScalar(transformation[1][1]), SkScalar(transformation[2][0]), SkScalar(transformation[2][1]) };
    SkMatrix matrix;
    matrix.setAffine(affine);
    return matrix;
}

static SkMatrix makeMatrix(const double transformation[6]) {
    SkScalar affine[6] = { SkScalar(transformation[0]), SkScalar(transformation[1]), SkScalar(transformation[2]), SkScalar(transformation[3]), SkScalar(transformation[4]), SkScalar(transformation[5]) };
    SkMatrix matrix;
    matrix.setAffine(affine);
    return matrix;
}

static bool makeSubshape(SkPath &path, const octopus::Path &octopusPath, SkPathFillType fillType) {
    switch (octopusPath.type) {
        case octopus::Path::Type::PATH:
            if (!(octopusPath.geometry.has_value() && SkParsePath::FromSVGString(octopusPath.geometry.value().c_str(), &path)))
                return false;
            path.setFillType(fillType);
            break;
        case octopus::Path::Type::RECTANGLE:
            if (!octopusPath.rectangle.has_value())
                return false;
            {
                SkRect rect = SkRect::MakeLTRB(
                    SkScalar(octopusPath.rectangle->x0),
                    SkScalar(octopusPath.rectangle->y0),
                    SkScalar(octopusPath.rectangle->x1),
                    SkScalar(octopusPath.rectangle->y1)
                );
                if (octopusPath.cornerRadius.has_value()) {
                    SkScalar radius = octopusPath.cornerRadius.value();
                    SkScalar radii[] = {
                        radius, radius, radius, radius,
                        radius, radius, radius, radius
                    };
                    path.addRoundRect(rect, radii);
                } else
                    path.addRect(rect);
            }
            break;
        case octopus::Path::Type::COMPOUND:
            if (octopusPath.paths.has_value()) {
                if (!octopusPath.op.has_value()) {
                    for (int i = 0; i < int(octopusPath.paths->size()); ++i) {
                        if (octopusPath.paths.value()[i].visible) {
                            SkPath p;
                            if (!(makeSubshape(p, octopusPath.paths.value()[i], fillType)))
                                return false;
                            path.addPath(p);
                        }
                    }
                } else {
                    SkPathOp op = kXOR_SkPathOp;
                    switch (octopusPath.op.value()) {
                        case octopus::Path::Op::UNION:
                            op = kUnion_SkPathOp;
                            break;
                        case octopus::Path::Op::INTERSECT:
                            op = kIntersect_SkPathOp;
                            break;
                        case octopus::Path::Op::SUBTRACT:
                            op = kDifference_SkPathOp;
                            break;
                        case octopus::Path::Op::EXCLUDE:
                            op = kXOR_SkPathOp;
                            break;
                    }
                    int i = 0;
                    for (; i < int(octopusPath.paths->size()); ++i) {
                        if (octopusPath.paths.value()[i].visible) {
                            if (!makeSubshape(path, octopusPath.paths.value()[i], fillType))
                                return false;
                            break;
                        }
                    }
                    for (++i; i < int(octopusPath.paths->size()); ++i) {
                        if (octopusPath.paths.value()[i].visible) {
                            SkPath b;
                            if (!(makeSubshape(b, octopusPath.paths.value()[i], fillType) && Op(path, b, op, &path)))
                                return false;
                        }
                    }
                }
            }
        break;
    }
    path.transform(makeMatrix(octopusPath.transform));
    return true;
}

static SkPathFillType pathFillType(const nonstd::optional<octopus::Shape::FillRule> &fillRule) {
    switch (fillRule.value_or(octopus::Shape::FillRule::EVEN_ODD)) {
        case octopus::Shape::FillRule::EVEN_ODD:
            return SkPathFillType::kEvenOdd;
        case octopus::Shape::FillRule::NON_ZERO:
            return SkPathFillType::kWinding;
    }
    return SkPathFillType::kEvenOdd;
}

static bool initShape(Rasterizer::Shape &shape) {
    if (shape.octopusShape.path.has_value() && shape.octopusShape.path->visible) {
        SkPathFillType fillType = pathFillType(shape.octopusShape.fillRule);
        if (!makeSubshape(shape.bodyPath, shape.octopusShape.path.value(), fillType))
            return false;
        shape.bodyPath.setFillType(fillType);
    }
    shape.strokePaths.resize(shape.octopusShape.strokes.size());
    for (size_t i = 0; i < shape.octopusShape.strokes.size(); ++i) {
        if (shape.octopusShape.strokes[i].path.has_value() && shape.octopusShape.strokes[i].path->visible) {
            SkPathFillType fillType = pathFillType(shape.octopusShape.strokes[i].fillRule);
            if (!makeSubshape(shape.strokePaths[i], shape.octopusShape.strokes[i].path.value(), fillType))
                return false;
            shape.strokePaths[i].setFillType(fillType);
        }
    }
    return true;
}

static void strokeToPaint(SkPaint &paint, const octopus::VectorStroke &stroke) {
    paint.setStyle(SkPaint::kStroke_Style);
    if (stroke.position == octopus::Stroke::Position::CENTER)
        paint.setStrokeWidth(SkScalar(stroke.thickness));
    else
        paint.setStrokeWidth(SkScalar(2*stroke.thickness));
    switch (stroke.lineJoin.value_or(defaultLineJoin)) {
        case octopus::VectorStroke::LineJoin::MITER:
            paint.setStrokeJoin(SkPaint::kMiter_Join);
            break;
        case octopus::VectorStroke::LineJoin::ROUND:
            paint.setStrokeJoin(SkPaint::kRound_Join);
            break;
        case octopus::VectorStroke::LineJoin::BEVEL:
            paint.setStrokeJoin(SkPaint::kBevel_Join);
            break;
    }
    switch (stroke.lineCap.value_or(defaultLineCap)) {
        case octopus::VectorStroke::LineCap::BUTT:
            paint.setStrokeCap(SkPaint::kButt_Cap);
            break;
        case octopus::VectorStroke::LineCap::ROUND:
            paint.setStrokeCap(SkPaint::kRound_Cap);
            break;
        case octopus::VectorStroke::LineCap::SQUARE:
            paint.setStrokeCap(SkPaint::kSquare_Cap);
            break;
    }
    paint.setStrokeMiter(SkScalar(stroke.miterLimit.value_or(defaultMiterLimit)));
    if (stroke.style.has_value() && stroke.style.value() != octopus::VectorStroke::Style::SOLID) {
        if (stroke.dashing.has_value()) {
            std::vector<SkScalar> dashing;
            dashing.resize(stroke.dashing->size());
            for (int i = 0; i < int(dashing.size()); ++i)
                dashing[i] = SkScalar(stroke.dashing.value()[i]);
            paint.setPathEffect(SkDashPathEffect::Make(dashing.data(), int(dashing.size()), SkScalar(stroke.dashOffset.value_or(0.))));
        } else {
            SkScalar dashing[2] = { };
            switch (stroke.style.value()) {
                case octopus::VectorStroke::Style::SOLID:
                    break;
                case octopus::VectorStroke::Style::DASHED:
                    dashing[0] = dashing[1] = 4;
                    break;
                case octopus::VectorStroke::Style::DOTTED:
                    dashing[0] = dashing[1] = 1;
                    break;
            }
            paint.setPathEffect(SkDashPathEffect::Make(dashing, 2, SkScalar(stroke.dashOffset.value_or(0.))));
        }
    }
}

Rasterizer::ShapePtr Rasterizer::createShape(const octopus::Shape &octopusShape, int flags) {
    ShapePtr shape(new Shape);
    shape->octopusShape = octopusShape;
    if (initShape(*shape))
        return shape;
    return nullptr;
}

bool Rasterizer::modifyShape(Shape *shape, const octopus::Shape &octopusShape, int flags) {
    ODE_ASSERT(shape);
    shape->octopusShape = octopusShape;
    if (!initShape(*shape))
        return false;
    return true;
}

Rectangle<double> Rasterizer::getBounds(Shape *shape, int strokeIndex, const Matrix3x2d &transformation) {
    ODE_ASSERT(shape);
    const SkPath *path = &shape->bodyPath;
    const octopus::VectorStroke *stroke = nullptr;
    if (strokeIndex >= 0) {
        if (strokeIndex < int(shape->octopusShape.strokes.size())) {
            if (shape->octopusShape.strokes[strokeIndex].path.has_value())
                path = &shape->strokePaths[strokeIndex];
            else
                stroke = &shape->octopusShape.strokes[strokeIndex];
        } else
            return Rectangle<double>::unspecified;
    }
    SkRect rect;
    Rectangle<double> bounds;
    if (transformation[0][1] || transformation[1][0]) { // otherwise only final bounds can be transformed
        SkPath transformedPath;
        path->transform(makeMatrix(transformation), &transformedPath, SkApplyPerspectiveClip::kNo);
        rect = transformedPath.computeTightBounds();
        bounds.a = Vector2d(double(rect.fLeft), double(rect.fTop));
        bounds.b = Vector2d(double(rect.fRight), double(rect.fBottom));
    } else {
        rect = path->computeTightBounds();
        Vector2d scale(transformation[0][0], transformation[1][1]);
        bounds.a = scale*Vector2d(double(rect.fLeft), double(rect.fTop))+transformation[2];
        bounds.b = scale*Vector2d(double(rect.fRight), double(rect.fBottom))+transformation[2];
    }
    if (stroke) {
        double padding = stroke->thickness;
        switch (stroke->position) {
            case octopus::Stroke::Position::CENTER:
                padding *= .5;
            case octopus::Stroke::Position::OUTSIDE:
                bounds += RectangleMargin<double>(padding);
                break;
            case octopus::Stroke::Position::INSIDE:
                break;
        }
    }
    return bounds;
}

#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
GrDirectContext * Rasterizer::Internal::getGraphicsContext() {
    if (!graphicsContext)
        graphicsContext = GrDirectContext::MakeGL(GrGLMakeNativeInterface());
    return graphicsContext.get();
}
#endif

bool Rasterizer::Internal::rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, SkSurface *surface) {
    const SkPath *path = &shape->bodyPath;
    const octopus::VectorStroke *stroke = nullptr;
    if (strokeIndex >= 0) {
        if (strokeIndex < int(shape->octopusShape.strokes.size())) {
            if (shape->octopusShape.strokes[strokeIndex].path.has_value())
                path = &shape->strokePaths[strokeIndex];
            else
                stroke = &shape->octopusShape.strokes[strokeIndex];
        } else
            return false;
    }
    SkCanvas *canvas = surface->getCanvas();
    canvas->setMatrix(makeMatrix(transformation));
    SkPaint paint;
    paint.setAntiAlias(true);
    if (stroke)
        strokeToPaint(paint, *stroke);
    else
        paint.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(*path, paint);
    return true;
}

bool Rasterizer::rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, const BitmapRef &dstBitmap) {
    ODE_ASSERT(shape && (dstBitmap || !dstBitmap.dimensions));
    if (pixelChannels(dstBitmap.format) != 1)
        return false;
    SkImageInfo imageInfo = SkImageInfo::MakeA8(dstBitmap.dimensions.x, dstBitmap.dimensions.y);
    if (imageInfo.minRowBytes() > dstBitmap.dimensions.x)
        return false;
    sk_sp<SkSurface> surface = SkSurface::MakeRasterDirect(imageInfo, dstBitmap.pixels, dstBitmap.dimensions.x);
    return data->rasterize(shape, strokeIndex, transformation, surface.get());
}

bool Rasterizer::rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, const TextureDescriptor &dstTexture) {
    #ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
        ODE_ASSERT(shape && dstTexture.handle && dstTexture.format != PixelFormat::EMPTY && dstTexture.dimensions.x > 0 && dstTexture.dimensions.y > 0);
        // TODO support other pixel types?
        ODE_ASSERT(dstTexture.format == PixelFormat::RGBA);
        if (dstTexture.format != PixelFormat::RGBA)
            return false;
        if (GrDirectContext *context = data->getGraphicsContext()) {
            context->resetContext();
            GrGLTextureInfo textureInfo = { };
            textureInfo.fTarget = GL_TEXTURE_2D;
            textureInfo.fID = dstTexture.handle;
            textureInfo.fFormat = GL_RGBA8;
            GrBackendTexture backendTexture(dstTexture.dimensions.x, dstTexture.dimensions.y, GrMipMapped::kNo, textureInfo);
            sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(context, backendTexture, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin, 0, SkColorType::kRGBA_8888_SkColorType, SkColorSpace::MakeSRGBLinear(), nullptr);
            surface->getCanvas()->clear(SkColor(0));
            bool result = data->rasterize(shape, strokeIndex, transformation, surface.get());
            auto texture = surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess);
            //surface->flushAndSubmit(); // not needed?
            // Restore ODE's OpenGL state
            glDisable(GL_BLEND);
            glDisable(GL_SCISSOR_TEST);
            return result;
        }
    #endif
    return false;
}

}
