
#include "Renderer.h"

#include <cstring>
#include <algorithm>
#include "GradientTexture.h"

namespace ode {

static Color animationFillColor(Component &component, const LayerInstanceSpecifier &layer, double time, Color color) {
    if (Result<const DocumentAnimation *, DesignError> anims = component.getAnimation(layer->id)) {
        ODE_ASSERT(anims.value());
        for (const LayerAnimation &animation : anims.value()->animations) {
            if (animation.type == LayerAnimation::FILL_COLOR) {
                color = animateColor(animation, time);
            }
        }
    }
    return color;
}

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

Renderer::Renderer(GraphicsContext &gc) : gc(gc), effectRenderer(gc, tfbManager, billboard, blitShader), compositingShaderRes(CompositingShader::prepare()), fillShaderRes(FillShader::prepare()) {
    const float billboardVertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
    int attributeSize = 2;
    billboard.initialize(billboardVertices, &attributeSize, 1, GL_TRIANGLES, 6);

    int pels[4] = { };
    transparentTexture.initialize(pels, 1, 1, PixelFormat::RGBA);
    memset(pels, 0xff, sizeof(pels));
    whiteTexture.initialize(pels, 1, 1, PixelFormat::RGBA);

    solidColorShader.initialize(compositingShaderRes);
    blitShader.initialize(compositingShaderRes);
    mixShader.initialize(compositingShaderRes);
    mixMaskShader.initialize(compositingShaderRes);
    alphaMultShader.initialize(compositingShaderRes);

    // TODO DELETE:
    transformShader.initialize();
}

PlacedImagePtr Renderer::blend(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode, bool ignoreSrcAlpha) {
    if (!dst)
        return src;
    if (!src)
        return dst;

    BlendShader &shader = blendShaders[blendMode];
    if (!shader.ready()) {
        if (!shader.initialize(compositingShaderRes, BlendShader::blendFunctionSource(blendMode))) {
            // TODO log error
            return nullptr;
        }
    }

    ScaledBounds bounds = dst.bounds()|src.bounds();
    if (!bounds)
        return nullptr;
    PixelBounds pxBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr dstTex = dst->asTexture();
    TexturePtr srcTex = src->asTexture();

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    shader.bind(pxBounds, bounds, dst.bounds(), src.bounds(), ignoreSrcAlpha);
    dstTex->bind(BlendShader::UNIT_DST);
    srcTex->bind(BlendShader::UNIT_SRC);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

PlacedImagePtr Renderer::blend(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode) {
    return blend(dst, src, blendMode, false);
}

PlacedImagePtr Renderer::blendIgnoreAlpha(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode) {
    return blend(dst, src, blendMode, true);
}

PlacedImagePtr Renderer::mask(const PlacedImagePtr &image, const PlacedImagePtr &mask, ChannelMatrix channelMatrix) {
    if (!image || !mask)
        return nullptr;

    ScaledBounds bounds = image.bounds()&mask.bounds();
    if (!bounds)
        return nullptr;
    PixelBounds pxBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr imageTex = image->asTexture();
    TexturePtr maskTex = mask->asTexture();

    if (mask->transparencyMode() == Image::RED_IS_ALPHA) {
        channelMatrix.m[4] += channelMatrix.m[0];
        channelMatrix.m[4] += channelMatrix.m[1];
        channelMatrix.m[4] += channelMatrix.m[2];
        channelMatrix.m[0] = channelMatrix.m[3];
        channelMatrix.m[1] = 0;
        channelMatrix.m[2] = 0;
        channelMatrix.m[3] = 0;
    }

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mixMaskShader.bind(pxBounds, bounds, bounds, image.bounds(), mask.bounds(), channelMatrix);
    transparentTexture.bind(MixMaskShader::UNIT_A);
    imageTex->bind(MixMaskShader::UNIT_B);
    maskTex->bind(MixMaskShader::UNIT_MASK);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

PlacedImagePtr Renderer::mixMask(const PlacedImagePtr &a, const PlacedImagePtr &b, const PlacedImagePtr &mask, ChannelMatrix channelMatrix) {
    if (!mask)
        return mix(a, b, channelMatrix.m[4]);
    if (!a)
        return this->mask(b, mask, channelMatrix);
    if (!b) {
        channelMatrix.m[0] = -channelMatrix.m[0];
        channelMatrix.m[1] = -channelMatrix.m[1];
        channelMatrix.m[2] = -channelMatrix.m[2];
        channelMatrix.m[3] = -channelMatrix.m[3];
        channelMatrix.m[4] = 1-channelMatrix.m[4];
        return this->mask(a, mask, channelMatrix);
    }

    ScaledBounds bounds = a.bounds()|b.bounds();
    if (!bounds)
        return nullptr;
    PixelBounds pxBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr aTex = a->asTexture();
    TexturePtr bTex = b->asTexture();
    TexturePtr maskTex = mask->asTexture();

    if (mask->transparencyMode() == Image::RED_IS_ALPHA) {
        channelMatrix.m[4] += channelMatrix.m[0];
        channelMatrix.m[4] += channelMatrix.m[1];
        channelMatrix.m[4] += channelMatrix.m[2];
        channelMatrix.m[0] = channelMatrix.m[3];
        channelMatrix.m[1] = 0;
        channelMatrix.m[2] = 0;
        channelMatrix.m[3] = 0;
    }

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mixMaskShader.bind(pxBounds, bounds, a.bounds(), b.bounds(), mask.bounds(), channelMatrix);
    aTex->bind(MixMaskShader::UNIT_A);
    bTex->bind(MixMaskShader::UNIT_B);
    maskTex->bind(MixMaskShader::UNIT_MASK);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

PlacedImagePtr Renderer::mix(const PlacedImagePtr &a, const PlacedImagePtr &b, double ratio) {
    if (ratio == 0)
        return a;
    if (ratio == 1)
        return b;
    if (!a)
        return multiplyAlpha(b, ratio);
    if (!b)
        return multiplyAlpha(a, 1-ratio);

    ScaledBounds bounds = a.bounds()|b.bounds();
    if (!bounds)
        return nullptr;
    PixelBounds pxBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr aTex = a->asTexture();
    TexturePtr bTex = b->asTexture();

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mixShader.bind(pxBounds, bounds, a.bounds(), b.bounds(), float(ratio));
    aTex->bind(MixShader::UNIT_A);
    bTex->bind(MixShader::UNIT_B);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

PlacedImagePtr Renderer::multiplyAlpha(const PlacedImagePtr &image, double multiplier) {
    if (multiplier == 0 || !image)
        return nullptr;
    if (multiplier == 1)
        return image;

    PixelBounds pxBounds = outerPixelBounds(image.bounds());
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr tex = image->asTexture();

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    alphaMultShader.bind(pxBounds, image.bounds(), image.bounds(), float(multiplier));
    tex->bind(AlphaMultShader::UNIT_IN);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

PlacedImagePtr Renderer::drawLayerBody(Component &component, const LayerInstanceSpecifier &layer, double scale, double time) {
    return drawLayerVector(component, layer, Rasterizer::BODY, scale, time);
}

PlacedImagePtr Renderer::drawLayerStroke(Component &component, const LayerInstanceSpecifier &layer, int index, double scale, double time) {
    return drawLayerVector(component, layer, index, scale, time);
}

PlacedImagePtr Renderer::drawLayerFill(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, double scale, double time) {
    if (layer->shape.has_value() && index < int(layer->shape->fills.size()))
        return drawFill(component, layer, imageBase, layer->shape->fills[index], scale, time);
    return nullptr;
}

PlacedImagePtr Renderer::drawLayerStrokeFill(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, double scale, double time) {
    if (layer->shape.has_value() && index < int(layer->shape->strokes.size()))
        return drawFill(component, layer, imageBase, layer->shape->strokes[index].fill, scale, time);
    return nullptr;
}

PlacedImagePtr Renderer::drawLayerText(Component &component, const LayerInstanceSpecifier &layer, double scale, double time) {
    // TODO new text rendering
    if (Result<textify::TextShapeHandle, DesignError> shape = component.getLayerTextShape(layer->id)) {
        textify::Dimensions dimensions = textify::getDrawBufferDimensions(TEXTIFY_CONTEXT, shape.value());
        // Hack to add 1 pixel padding for clamp-to-edge, in-place pixel move
        BitmapPtr bitmap(new Bitmap(PixelFormat::PREMULTIPLIED_RGBA, Vector2i(dimensions.width+2, dimensions.height+2)));
        size_t pxSize = pixelSize(bitmap->format());
        size_t bitmapStride = pxSize*bitmap->width();
        bitmap->clear();
        BitmapRef misalignedBitmap(bitmap->format(), (*bitmap)(1, 1), dimensions.width, dimensions.height);
        SparseBitmapRef bitmapMid(bitmap->format(), (*bitmap)(1, 1), dimensions.width, dimensions.height, bitmapStride);
        textify::DrawTextResult result = textify::drawText(TEXTIFY_CONTEXT, shape.value(), misalignedBitmap.pixels, misalignedBitmap.width(), misalignedBitmap.height());
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
        Matrix3x3d imageTransform = Matrix3x3d(TransformationMatrix::scale(scale)*layer.parentTransform*TransformationMatrix(layer->transform)*animationTransform(component, layer, time))*fromTextifyMatrix(result.transform);
        return transformImage(image, imageTransform);
    }
    return nullptr;
}

PlacedImagePtr Renderer::drawLayerEffect(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, const PlacedImagePtr &basis, double scale, double time) {
    ODE_ASSERT(index >= 0 && index < (int) layer->effects.size());
    const octopus::Effect &effect = layer->effects[index];
    if (effect.type == octopus::Effect::Type::OVERLAY) {
        if (effect.overlay.has_value())
            return drawFill(component, layer, imageBase, effect.overlay.value(), scale, time);
    } else
        return effectRenderer.drawEffect(effect, basis, scale*layer.parentFeatureScale*layer->featureScale.value_or(1));
    return nullptr;
}

PlacedImagePtr Renderer::applyFilter(const octopus::Filter &filter, const PlacedImagePtr &basis) {
    // TODO
    return basis;
}

PlacedImagePtr Renderer::reframe(const PlacedImagePtr &image, const PixelBounds &bounds) {
    ScaledBounds sBounds((Vector2d) bounds.a, (Vector2d) bounds.b);
    TextureFrameBufferPtr outTex = tfbManager.acquireExact(bounds);
    TexturePtr tex = image ? image->asTexture() : nullptr;

    outTex->bind();
    glViewport(0, 0, bounds.dimensions().x, bounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (tex) {
        blitShader.bind(bounds-bounds.a, sBounds, image.bounds());
        tex->bind(BlitShader::UNIT_IN);
        billboard.draw();
    }
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), bounds);
}

void Renderer::screenDraw(const PixelBounds &viewport, const PlacedImagePtr &image, const Color &bgColor) {
    if (!image)
        return;
    TexturePtr tex = image->asTexture();
    if (!tex)
        return; // TODO log error

    gc.bindOutputFramebuffer();
    glViewport(viewport.a.x, viewport.a.y, viewport.dimensions().x, viewport.dimensions().y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(float(bgColor.r), float(bgColor.g), float(bgColor.b), float(bgColor.a));
    glClear(GL_COLOR_BUFFER_BIT);
    PixelBounds bottomUpViewport = viewport;
    bottomUpViewport.a.y = viewport.b.y;
    bottomUpViewport.b.y = viewport.a.y;
    blitShader.bind(bottomUpViewport, image.bounds(), image.bounds());
    tex->bind(AlphaMultShader::UNIT_IN);
    billboard.draw();
    glDisable(GL_BLEND);
}

void Renderer::cleanUp() {
    tfbManager = TextureFrameBufferManager();
}

// TODO DEPRECATE
PlacedImagePtr Renderer::resolveAlphaChannel(const PlacedImagePtr &image) {
    if (!(image && image.bounds()))
        return nullptr;
    if (image->transparencyMode() != Image::RED_IS_ALPHA)
        return image;

    PixelBounds pxBounds = outerPixelBounds(image.bounds());
    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);

    TexturePtr maskTex = image->asTexture();

    outTex->bind();
    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mixMaskShader.bind(pxBounds, image.bounds(), image.bounds(), image.bounds(), image.bounds(), ChannelMatrix { { 1, 0, 0, 0, 0 } });
    transparentTexture.bind(MixMaskShader::UNIT_A);
    whiteTexture.bind(MixMaskShader::UNIT_B);
    maskTex->bind(MixMaskShader::UNIT_MASK);
    billboard.draw();
    outTex->unbind();
    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
}

// THE REST IS TEMPORARILY BORROWED FROM OLD RENDERER - TODO REMAKE

static Matrix3x3d translationMatrix(const Vector2i &v) {
    return Matrix3x3d(1, 0, 0, 0, 1, 0, v.x, v.y, 1);
}

static Matrix3x3d scaleMatrix(const Vector2d &v) {
    return Matrix3x3d(v.x, 0, 0, 0, v.y, 0, 0, 0, 1);
}

static byte cComp_f2b(float v) {
    return (byte) std::min(std::max(int(256.f*v), 0x00), 0xff);
}

static UnscaledBounds transformBounds(const UntransformedBounds &bounds, const TransformationMatrix &matrix) {
    Vector2d a = matrix*Vector3d(bounds.a.x, bounds.a.y, 1);
    Vector2d b = matrix*Vector3d(bounds.b.x, bounds.a.y, 1);
    Vector2d c = matrix*Vector3d(bounds.a.x, bounds.b.y, 1);
    Vector2d d = matrix*Vector3d(bounds.b.x, bounds.b.y, 1);
    return UnscaledBounds(
        std::min(std::min(std::min(a.x, b.x), c.x), d.x),
        std::min(std::min(std::min(a.y, b.y), c.y), d.y),
        std::max(std::max(std::max(a.x, b.x), c.x), d.x),
        std::max(std::max(std::max(a.y, b.y), c.y), d.y)
    );
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

PlacedImagePtr Renderer::transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation) {

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

PlacedImagePtr Renderer::drawLayerVector(Component &component, const LayerInstanceSpecifier &layer, int strokeIndex, double scale, double time) {
    if (Result<Rasterizer::Shape *, DesignError> shape = component.getLayerShape(layer->id)) {
        if (Result<LayerBounds, DesignError> layerBounds = component.getLayerBounds(layer->id)) {
            TransformationMatrix animationMatrix = animationTransform(component, layer, time);
            TransformationMatrix layerTransform = TransformationMatrix::scale(scale)*layer.parentTransform*TransformationMatrix(layer->transform)*animationMatrix;
            if (PixelBounds bounds = outerPixelBounds(scaleBounds(transformBounds(layerBounds.value().untransformedBounds, layerTransform), 1))) {
                // A transparent margin of 1 pixel on each side is added to make sure that CLAMP_TO_EDGE extends with transparent color
                bounds += PixelMargin(1);
                #ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
                    TextureFrameBufferPtr texture = tfbManager.acquire(bounds);
                    Matrix3x2d transformation = TransformationMatrix(1, 0, 0, 1, -bounds.a.x, -bounds.a.y)*layerTransform; // do not move before ifdef, bounds modified in previous statement
                    Rasterizer::TextureDescriptor textureDescriptor = { };
                    textureDescriptor.handle = texture->getInternalGLHandle();
                    textureDescriptor.dimensions = texture->dimensions();
                    textureDescriptor.format = texture->format();
                    if (rasterizer.rasterize(shape.value(), strokeIndex, transformation, textureDescriptor))
                        return PlacedImagePtr(ImagePtr(new TextureImage(texture, Image::NORMAL, Image::NO_BORDER)), bounds);
                #else
                    Matrix3x2d transformation = TransformationMatrix(1, 0, 0, 1, -bounds.a.x, -bounds.a.y)*layerTransform;
                    BitmapPtr bitmap(new Bitmap(PixelFormat::ALPHA, bounds.dimensions()));
                    bitmap->clear();
                    if (rasterizer.rasterize(shape.value(), strokeIndex, transformation, *bitmap)) {
                        bitmap->reinterpret(PixelFormat::R);
                        return PlacedImagePtr(ImagePtr(new BitmapImage((BitmapPtr &&) bitmap, Image::RED_IS_ALPHA, Image::NO_BORDER)), bounds);
                    }
                #endif
            }
        }
    }
    return nullptr;
}

PlacedImagePtr Renderer::drawFill(Component &component, const LayerInstanceSpecifier &layer, ImageBase &imageBase, const octopus::Fill &fill, double scale, double time) {
    if (Result<LayerBounds, DesignError> layerBounds = component.getLayerBounds(layer->id)) {
        TransformationMatrix animationMatrix = animationTransform(component, layer, time);
        TransformationMatrix layerTransform = layer.parentTransform*TransformationMatrix(layer->transform)*animationMatrix;
        UnscaledBounds fillBounds = transformBounds(layerBounds.value().untransformedBounds, layerTransform);
        ScaledBounds sFillBounds = scaleBounds(fillBounds, scale);

        TransformationMatrix transform;
        if (fill.positioning.has_value()) {
            const octopus::Fill::Positioning &positioning = fill.positioning.value();
            transform = TransformationMatrix(positioning.transform);
            switch (positioning.origin) {
                case octopus::Fill::Positioning::Origin::ARTBOARD:
                case octopus::Fill::Positioning::Origin::COMPONENT:
                    break;
                case octopus::Fill::Positioning::Origin::PARENT:
                    transform = layer.parentTransform*transform; // TODO animationTransform of parent layer
                    break;
                case octopus::Fill::Positioning::Origin::LAYER:
                    transform = layer.parentTransform*TransformationMatrix(layer->transform)*animationMatrix*transform;
                    break;
            }
            transform = TransformationMatrix::scale(scale)*transform;
        }

        switch (fill.type) {
            case octopus::Fill::Type::COLOR:
                if (fill.color.has_value()) {
                    Color color = animationFillColor(component, layer, time, Color(fill.color->r, fill.color->g, fill.color->b, fill.color->a));
                    TextureFrameBufferPtr t = tfbManager.acquireExact(PixelBounds(0, 0, 1, 1));
                    t->bind();
                    glClearColor(GLclampf(color.r*color.a), GLclampf(color.g*color.a), GLclampf(color.b*color.a), GLclampf(color.a));
                    glClear(GL_COLOR_BUFFER_BIT);
                    t->unbind();
                    return PlacedImagePtr(Image::fromTexture(t, Image::NORMAL), sFillBounds+ScaledMargin(1));
                }
                break;

            case octopus::Fill::Type::GRADIENT:
                if (fill.gradient.has_value()) {
                    GradientFillShader &shader = gradientFillShaders[fill.gradient->type];
                    if (!shader.ready()) {
                        if (!shader.initialize(fillShaderRes, GradientFillShader::shapeFunctionSource(fill.gradient->type))) {
                            // TODO log error
                            return nullptr;
                        }
                    }

                    GradientTexture gradientTexture;
                    if (!gradientTexture.initialize(fill.gradient.value())) {
                        // TODO log error
                        return nullptr;
                    }

                    ScaledBounds bounds = sFillBounds+ScaledMargin(1);
                    PixelBounds pxBounds = outerPixelBounds(bounds);
                    TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);
                    outTex->bind();
                    glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
                    shader.bind(pxBounds, bounds, Matrix3x3f(Matrix3x3d(transform)), gradientTexture.transformation());
                    gradientTexture.bind(GradientFillShader::UNIT_GRADIENT);
                    billboard.draw();
                    outTex->unbind();
                    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
                }
                break;

            case octopus::Fill::Type::IMAGE:
                if (fill.image.has_value()) {
                    ImageFillShader &shader = imageFillShaders[0];
                    if (!shader.ready()) {
                        if (!shader.initialize(fillShaderRes)) {
                            // TODO log error
                            return nullptr;
                        }
                    }

                    if (ImagePtr image = imageBase.get(fill.image.value())) {
                        TexturePtr texture = image->asTexture();
                        if (!texture) {
                            // TODO log error
                            return nullptr;
                        }
                        Vector2i imageDims = image->dimensions();
                        if (image->borderMode() == Image::ONE_PIXEL_BORDER)
                            imageDims -= Vector2i(2);

                        if (fill.positioning.has_value()) {
                            const octopus::Fill::Positioning &positioning = fill.positioning.value();
                            switch (positioning.layout) {
                                case octopus::Fill::Positioning::Layout::STRETCH:
                                case octopus::Fill::Positioning::Layout::TILE:
                                    break;
                                case octopus::Fill::Positioning::Layout::FILL:
                                case octopus::Fill::Positioning::Layout::FIT: {
                                    Vector2d targetDims(
                                        (transform*Vector3d(1, 0, 0)).length(),
                                        (transform*Vector3d(0, 1, 0)).length()
                                    );
                                    double aspectRatio = imageDims.x*targetDims.y/(imageDims.y*targetDims.x);
                                    Vector2d aspectScale(1);
                                    switch (positioning.layout) {
                                        case octopus::Fill::Positioning::Layout::FILL:
                                            aspectScale.x = std::max(aspectRatio, 1.);
                                            aspectScale.y = std::max(1/aspectRatio, 1.);
                                            break;
                                        case octopus::Fill::Positioning::Layout::FIT:
                                            aspectScale.x = std::min(aspectRatio, 1.);
                                            aspectScale.y = std::min(1/aspectRatio, 1.);
                                            break;
                                        default:
                                            ODE_ASSERT(!"Should be unreachable");
                                    }
                                    transform *= TransformationMatrix::scale(aspectScale, Vector2d(.5));
                                    break;
                                }
                            }
                        }
                        if (image->borderMode() == Image::ONE_PIXEL_BORDER) {
                            ODE_ASSERT(imageDims.x > 0 && imageDims.y > 0);
                            transform *= TransformationMatrix(
                                (double) (imageDims.x+2)/imageDims.x, 0,
                                0, (double) (imageDims.y+2)/imageDims.y,
                                -1./imageDims.x, -1./imageDims.y
                            );
                        }

                        ScaledBounds bounds = sFillBounds+ScaledMargin(1);
                        PixelBounds pxBounds = outerPixelBounds(bounds);
                        TextureFrameBufferPtr outTex = tfbManager.acquire(pxBounds);
                        outTex->bind();
                        glViewport(0, 0, pxBounds.dimensions().x, pxBounds.dimensions().y);
                        shader.bind(pxBounds, bounds, Matrix3x3f(Matrix3x3d(transform)));
                        texture->bind(ImageFillShader::UNIT_IMAGE);
                        billboard.draw();
                        outTex->unbind();
                        return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), pxBounds);
                    }
                }
                break;
        }
    }
    return nullptr;
}

}
