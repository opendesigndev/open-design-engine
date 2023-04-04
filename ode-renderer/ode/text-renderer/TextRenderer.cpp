
#include "TextRenderer.h"

#include <open-design-text-renderer/PlacedTextData.h>
#include "TextMesh.h"
#include "FontAtlas.h"

namespace ode {

// TODO delete when text transformations are implemented properly
static TransformationMatrix animationTransform(Component &component, const LayerInstanceSpecifier &layer, double time) {
    TransformationMatrix result = TransformationMatrix::identity;
    if (Result<const DocumentAnimation *, DesignError> anims = component.getAnimation(layer->id)) {
        ODE_ASSERT(anims.value());
        for (const LayerAnimation &animation : anims.value()->animations) {
            if (animation.type == LayerAnimation::TRANSFORM || animation.type == LayerAnimation::ROTATION) {
                result = animateTransform(animation, time)*result;
            }
        }
    }
    return result;
}

TextRenderer::TextRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard) : tfbManager(tfbManager), billboard(billboard) {
    #ifdef ODE_REALTIME_TEXT_RENDERER
        sdfShader.initialize();
    #else
        transformShader.initialize();
    #endif
}

TextRenderer::~TextRenderer() = default;

#ifdef ODE_REALTIME_TEXT_RENDERER

FontAtlas &TextRenderer::fontAtlas(const odtr::FontSpecifier &fontSpecifier) {
    return fontAtlases[fontSpecifier];
}

PlacedImagePtr TextRenderer::drawLayerText(Component &component, const LayerInstanceSpecifier &layer, const ScaledBounds &visibleBounds, double scale, double time) {
    if (Result<TextShapeHolder *, DesignError> shape = component.getLayerTextShape(layer->id)) {
        if (!(shape.value() && *shape.value()))
            return nullptr;
        if (!sdfShader.ready())
            return nullptr; // TODO report
        TextMesh *textMesh = static_cast<TextMesh *>(shape.value()->rendererData.get());
        if (!textMesh) {
            std::unique_ptr<TextMesh> textMeshHolder = TextMesh::build(*this, *shape.value());
            if (!textMeshHolder)
                return nullptr;
            textMesh = textMeshHolder.get();
            shape.value()->rendererData = std::move(textMeshHolder);
        }
        ODE_ASSERT(textMesh);

        const odtr::PlacedTextData *placedTextData = odtr::getShapedText(TEXT_RENDERER_CONTEXT, *shape.value());
        ODE_ASSERT(placedTextData);
        odtr::Dimensions dimensions = odtr::getDrawBufferDimensions(TEXT_RENDERER_CONTEXT, *shape.value());
        TransformationMatrix unscaledTransform = (
            layer.parentTransform*
            TransformationMatrix(layer->transform)*
            animationTransform(component, layer, time)*
            TransformationMatrix(Matrix3x2d(fromTextRendererMatrix(placedTextData->textTransform)))
        );
        ScaledBounds bounds = scaleBounds(transformBounds(UntransformedBounds(0, 0, dimensions.width, dimensions.height), unscaledTransform), scale)&visibleBounds;
        PixelBounds pxBounds = outerPixelBounds(bounds)+PixelMargin(1);
        TransformationMatrix transformation = TransformationMatrix::scale(scale)*unscaledTransform;

        TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);
        outTex->bind();
        glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        sdfShader.bind(pxBounds, Matrix3x3f(Matrix3x3d(transformation)));
        textMesh->draw(sdfShader.texCoordFactorUniform(), SDFTextShader::UNIT_IN);
        billboard.draw();
        outTex->unbind();
        return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
    }
    return nullptr;
}

#else

PlacedImagePtr TextRenderer::drawLayerText(Component &component, const LayerInstanceSpecifier &layer, const ScaledBounds &visibleBounds, double scale, double time) {
    if (Result<TextShapeHolder *, DesignError> shape = component.getLayerTextShape(layer->id)) {
        if (!(shape.value() && *shape.value()))
            return nullptr;
        odtr::Dimensions dimensions = odtr::getDrawBufferDimensions(TEXT_RENDERER_CONTEXT, *shape.value());
        // Hack to add 1 pixel padding for clamp-to-edge, in-place pixel move
        BitmapPtr bitmap(new Bitmap(PixelFormat::PREMULTIPLIED_RGBA, Vector2i(dimensions.width+2, dimensions.height+2)));
        size_t pxSize = pixelSize(bitmap->format());
        size_t bitmapStride = pxSize*bitmap->width();
        bitmap->clear();
        BitmapRef misalignedBitmap(bitmap->format(), (*bitmap)(1, 1), dimensions.width, dimensions.height);
        SparseBitmapRef bitmapMid(bitmap->format(), (*bitmap)(1, 1), dimensions.width, dimensions.height, bitmapStride);
        odtr::DrawTextResult result = odtr::drawText(TEXT_RENDERER_CONTEXT, *shape.value(), misalignedBitmap.pixels, misalignedBitmap.width(), misalignedBitmap.height());
        // Fix misaligned pixel rows
        for (int y = bitmapMid.height()-1; y > 0; --y) {
            memmove(bitmapMid(0, y), misalignedBitmap(0, y), pxSize*bitmapMid.width());
            // Re-clear leftmost and rightmost column
            memset((*bitmap)(0, y+1), 0, pxSize);
            memset((*bitmap)(bitmap->width()-1, y+1), 0, pxSize);
        }
        memset((*bitmap)(bitmap->width()-1, 1), 0, pxSize);
        ScaledBounds bounds(result.bounds.l, result.bounds.t, result.bounds.l+result.bounds.w, result.bounds.t+result.bounds.h);
        ScaledMargin padding;
        padding.a.x = padding.b.x = bounds.dimensions().x/dimensions.width;
        padding.a.y = padding.b.y = bounds.dimensions().y/dimensions.height;
        bounds += padding;
        PlacedImagePtr image(ImagePtr(new BitmapImage(bitmap, Image::NORMAL, Image::NO_BORDER)), bounds);
        Matrix3x3d imageTransform = Matrix3x3d(TransformationMatrix::scale(scale)*layer.parentTransform*TransformationMatrix(layer->transform)*animationTransform(component, layer, time))*fromTextRendererMatrix(result.transform);
        return transformImage(image, imageTransform);
    }
    return nullptr;
}

static ScaledBounds transformBounds(const ScaledBounds &bounds, const Matrix3x3d &matrix) {
    Vector3d a = matrix*Vector3d(bounds.a.x, bounds.a.y, 1);
    Vector3d b = matrix*Vector3d(bounds.b.x, bounds.a.y, 1);
    Vector3d c = matrix*Vector3d(bounds.a.x, bounds.b.y, 1);
    Vector3d d = matrix*Vector3d(bounds.b.x, bounds.b.y, 1);
    a /= a.z, b /= b.z, c /= c.z, d /= d.z;
    return ScaledBounds(
        std::min(std::min(std::min(a.x, b.x), c.x), d.x),
        std::min(std::min(std::min(a.y, b.y), c.y), d.y),
        std::max(std::max(std::max(a.x, b.x), c.x), d.x),
        std::max(std::max(std::max(a.y, b.y), c.y), d.y)
    );
}

PlacedImagePtr TextRenderer::transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation) {

    if (transformation[0][1] == 0 && transformation[0][2] == 0 && transformation[1][0] == 0 && transformation[1][2] == 0) {
        // transform placement only
        return PlacedImagePtr(image, transformBounds(image.bounds(), transformation));
    }

    Vector3 a = transformation*Vector3(image.bounds().a.x, image.bounds().a.y, 1.);
    Vector3 b = transformation*Vector3(image.bounds().a.x, image.bounds().b.y, 1.);
    Vector3 c = transformation*Vector3(image.bounds().b.x, image.bounds().a.y, 1.);
    Vector3 d = transformation*Vector3(image.bounds().b.x, image.bounds().b.y, 1.);
    ScaledBounds sOutBounds(
        std::min(std::min(std::min(a.x, b.x), c.x), d.x),
        std::min(std::min(std::min(a.y, b.y), c.y), d.y),
        std::max(std::max(std::max(a.x, b.x), c.x), d.x),
        std::max(std::max(std::max(a.y, b.y), c.y), d.y)
    );
    PixelBounds outputBounds = outerPixelBounds(sOutBounds);
    if (!outputBounds)
        return nullptr;
    if (!transformShader.ready())
        return nullptr; // TODO report
    TextureFrameBufferPtr outputTexture = tfbManager.acquire(outputBounds);

    Matrix3x3f toInputBounds(
        .5f*image.bounds().dimensions().x, 0.f, 0.f,
        0.f, .5f*image.bounds().dimensions().y, 0.f,
        .5f*image.bounds().dimensions().x+image.bounds().a.x, .5f*image.bounds().dimensions().y+image.bounds().a.y, 1.f
    );
    Matrix3x3f fromOutputBoundsToPolar(
        2.f/(float) outputBounds.dimensions().x, 0.f, 0.f,
        0.f, 2.f/(float) outputBounds.dimensions().y, 0.f,
        -2.f*(float) outputBounds.a.x/(float) outputBounds.dimensions().x-1.f, -2.f*(float) outputBounds.a.y/(float) outputBounds.dimensions().y-1.f, 1.f
    );
    Matrix3x3f vertexTransform = fromOutputBoundsToPolar*Matrix3x3f(transformation)*toInputBounds;

    TexturePtr tex = image->asTexture();

    tex->bind(0);
    outputTexture->bind();
    glViewport(0, 0, outputBounds.dimensions().x, outputBounds.dimensions().y);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    const int ss[] = { 1, 1 };
    transformShader.bind(vertexTransform, outputBounds.dimensions());
    billboard.draw();
    outputTexture->unbind();

    return PlacedImagePtr(Image::fromTexture(outputTexture, Image::NORMAL), outputBounds);
}

#endif

}
