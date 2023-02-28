
#include "EffectRenderer.h"

#include <algorithm>

namespace ode {

EffectRenderer::ShaderManager::ShaderManager() : shaderRes(EffectShader::prepare()) { }

BoundedBlurShader *EffectRenderer::ShaderManager::getBoundedBlurShader(char channel) {
    BoundedBlurShader *shader = nullptr;
    switch (channel) {
        case '\0':
            shader = &boundedBlurShaders[0];
            break;
        case 'a':
            shader = &boundedBlurShaders[1];
            break;
        case 'r':
            shader = &boundedBlurShaders[2];
            break;
        default:
            ODE_ASSERT(!"Shader for this channel is missing");
    }
    if (shader && (shader->ready() || shader->initialize(shaderRes, channel, EFFECT_SHADER_PRECISION)))
        return shader;
    return nullptr;
}

GaussianBlurShader *EffectRenderer::ShaderManager::getGaussianBlurShader() {
    if (gaussianBlurShader.ready() || gaussianBlurShader.initialize(shaderRes, '\0', EFFECT_SHADER_PRECISION))
        return &gaussianBlurShader;
    return nullptr;
}

DistanceTransformShader *EffectRenderer::ShaderManager::getDistanceTransformShader(char channel) {
    DistanceTransformShader *shader = nullptr;
    switch (channel) {
        case 'a':
            shader = &distanceTransformShaders[0];
            break;
        case 'r':
            shader = &distanceTransformShaders[1];
            break;
        default:
            ODE_ASSERT(!"Shader for this channel is missing");
    }
    if (shader && (shader->ready() || shader->initialize(shaderRes, channel, EFFECT_SHADER_PRECISION)))
        return shader;
    return nullptr;
}

DistanceThresholdShader *EffectRenderer::ShaderManager::getDistanceThresholdShader() {
    if (distanceThresholdShader.ready() || distanceThresholdShader.initialize(shaderRes, EFFECT_SHADER_PRECISION))
        return &distanceThresholdShader;
    return nullptr;
}

EffectRenderer::EffectRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard, BlitShader &blitShader) : gc(gc), tfbManager(tfbManager), billboard(billboard), blitShader(blitShader) { }

PlacedImagePtr EffectRenderer::drawEffect(const octopus::Effect &effect, const PlacedImagePtr &basis, double scale) {
    switch (effect.type) {
        case octopus::Effect::Type::OVERLAY:
            ODE_ASSERT(!"Should be handled by caller");
            return nullptr;
        case octopus::Effect::Type::STROKE:
            if (effect.stroke.has_value())
                return drawStroke(effect.stroke.value(), basis, scale);
            return nullptr;
        case octopus::Effect::Type::DROP_SHADOW:
        case octopus::Effect::Type::INNER_SHADOW:
            if (effect.shadow.has_value())
                return drawShadow(effect.type, effect.shadow.value(), basis, scale);
            return nullptr;
        case octopus::Effect::Type::OUTER_GLOW:
        case octopus::Effect::Type::INNER_GLOW:
            if (effect.glow.has_value())
                return drawShadow(effect.type, effect.glow.value(), basis, scale);
            return nullptr;
        case octopus::Effect::Type::BLUR:
            if (effect.blur.has_value())
                return drawGaussianBlur(scale*effect.blur.value(), basis);
            return nullptr;
        case octopus::Effect::Type::OTHER:
            return nullptr;
    }
    ODE_ASSERT(!"Incomplete switch");
    return nullptr;
}

PlacedImagePtr EffectRenderer::drawStroke(const octopus::Stroke &stroke, const PlacedImagePtr &basis, double scale) {
    if (!(stroke.fill.type == octopus::Fill::Type::COLOR && stroke.fill.color.has_value()))
        return nullptr;
    double thickness = scale*stroke.thickness;
    double margin;
    float minDistance, maxDistance;
    float lowerThreshold, upperThreshold;
    switch (stroke.position) {
        case octopus::Stroke::Position::OUTSIDE:
            margin = thickness+1;
            minDistance = (float) std::min(-thickness-1, 0.);
            maxDistance = (float) std::max(-thickness+1, 0.);
            lowerThreshold = (float) -thickness;
            upperThreshold = maxDistance+1.f;
            break;
        case octopus::Stroke::Position::CENTER:
            margin = .5*thickness+1;
            upperThreshold = (float) .5*thickness;
            lowerThreshold = -upperThreshold;
            minDistance = lowerThreshold-1.f;
            maxDistance = upperThreshold+1.f;
            break;
        case octopus::Stroke::Position::INSIDE:
            margin = 0;
            minDistance = (float) std::min(thickness-1, 0.);
            maxDistance = (float) std::max(thickness+1, 0.);
            lowerThreshold = minDistance-1.f;
            upperThreshold = (float) thickness;
            break;
    }
    return drawDistanceThreshold(minDistance, maxDistance, lowerThreshold, upperThreshold, fromOctopus(stroke.fill.color.value()), basis, basis.bounds()+ScaledMargin(margin));
}

PlacedImagePtr EffectRenderer::drawShadow(octopus::Effect::Type type, const octopus::Shadow &shadow, PlacedImagePtr basis, double scale) {
    if (!basis)
        return nullptr;
    bool inner = type == octopus::Effect::Type::INNER_SHADOW || type == octopus::Effect::Type::INNER_GLOW;
    ScaledBounds inputBounds = basis.bounds();
    Vector2d offset = scale*fromOctopus(shadow.offset);
    Vector2d inOffset = inner ? offset : Vector2d();
    Vector2d outOffset = inner ? Vector2d() : offset;
    double radius = fabs(scale*shadow.blur);
    double choke = scale*(inner ? -shadow.choke : shadow.choke);
    if (!radius) {
        if (choke) {
            basis = drawChoke(choke, inner ? Color(1) : fromOctopus(shadow.color), basis);
            if (!inner)
                return PlacedImagePtr(basis, basis.bounds()+outOffset);
        }

        // Basis to shadow color
        TexturePtr basisTex = basis->asTexture();
        if (!basisTex)
            return nullptr;
        ScaledBounds bounds = inner ? inputBounds : basis.bounds();
        PixelBounds pixelBounds = outerPixelBounds(bounds);
        TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
        ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion
        outTex->bind();
        glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
        // Fill outTex with shadow color
        glClearColor(
            shadow.color.a*shadow.color.r,
            shadow.color.a*shadow.color.g,
            shadow.color.a*shadow.color.b,
            shadow.color.a
        );
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, inner ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
        blitShader.bind(pixelBounds, actualBounds, basis.bounds()+inOffset);
        basisTex->bind(BlitShader::UNIT_IN);
        billboard.draw();
        glDisable(GL_BLEND);
        outTex->unbind();
        return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds+outOffset);
    }

    if (choke)
        basis = drawChoke(choke, Color(1), basis);
    BoundedBlurShader *blurShader = shaders.getBoundedBlurShader(basis->transparencyMode() == Image::RED_IS_ALPHA ? 'r' : 'a');
    if (!blurShader)
        return nullptr;

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds bounds = inner ? inputBounds : basis.bounds()+ScaledMargin(radius);
    PixelBounds intermediatePixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(intermediatePixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(intermediatePixelBounds, bounds, basis.bounds()+inOffset, false, radius, Color(1));
    basisTex->bind(BoundedBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds intermediateActualBounds(Vector2d(intermediatePixelBounds.a), Vector2d(intermediatePixelBounds.b)); // TODO proper conversion

    PixelBounds pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(pixelBounds, bounds, intermediateActualBounds, true, radius, inner ? Color(1) : fromOctopus(shadow.color));
    intermediateTex->bind(BoundedBlurShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    if (inner) {
        intermediateTex->bind();
        glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
        glClearColor(GLclampf(shadow.color.r*shadow.color.a), GLclampf(shadow.color.g*shadow.color.a), GLclampf(shadow.color.b*shadow.color.a), GLclampf(shadow.color.a));
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        blitShader.bind(intermediatePixelBounds, actualBounds, actualBounds);
        outTex->bind(BlitShader::UNIT_IN);
        billboard.draw();
        glDisable(GL_BLEND);
        outTex->unbind();
        outTex = (TextureFrameBufferPtr &&) intermediateTex;
        actualBounds = intermediateActualBounds;
    }

    return PlacedImagePtr(Image::fromTexture(outTex, basis->transparencyMode() == Image::RED_IS_ALPHA ? Image::RED_IS_ALPHA : Image::PREMULTIPLIED), actualBounds+outOffset);
}

PlacedImagePtr EffectRenderer::drawBoundedBlur(double blur, const PlacedImagePtr &basis) {
    if (!blur)
        return basis;
    if (!basis)
        return nullptr;
    ODE_ASSERT(basis->transparencyMode() == Image::PREMULTIPLIED || basis->transparencyMode() == Image::NO_TRANSPARENCY);
    BoundedBlurShader *blurShader = shaders.getBoundedBlurShader();
    if (!blurShader)
        return nullptr;

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds bounds = basis.bounds()+ScaledMargin(blur);
    PixelBounds pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(pixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(pixelBounds, bounds, basis.bounds(), false, blur, Color(1));
    basisTex->bind(BoundedBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(pixelBounds, bounds, actualBounds, true, blur, Color(1));
    intermediateTex->bind(BoundedBlurShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    actualBounds = ScaledBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds);
}

PlacedImagePtr EffectRenderer::drawGaussianBlur(double blur, const PlacedImagePtr &basis) {
    if (!blur)
        return basis;
    if (!basis)
        return nullptr;
    ODE_ASSERT(basis->transparencyMode() == Image::PREMULTIPLIED || basis->transparencyMode() == Image::NO_TRANSPARENCY);
    GaussianBlurShader *blurShader = shaders.getGaussianBlurShader();
    if (!blurShader)
        return nullptr;

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds bounds = basis.bounds()+ScaledMargin(GAUSSIAN_BLUR_MARGIN_FACTOR*blur);
    PixelBounds pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(pixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(pixelBounds, bounds, basis.bounds(), false, blur, Color(1));
    basisTex->bind(GaussianBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader->bind(pixelBounds, bounds, actualBounds, true, blur, Color(1));
    intermediateTex->bind(GaussianBlurShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    actualBounds = ScaledBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds);
}

PlacedImagePtr EffectRenderer::drawChoke(double choke, const Color &color, const PlacedImagePtr &basis) {
    ScaledBounds bounds = basis.bounds()+ScaledMargin(choke+1.01);
    if (!bounds)
        return nullptr;
    float minDistance = (float) std::min(-choke-1, 0.);
    float maxDistance = (float) std::max(-choke+1, 0.);
    return drawDistanceThreshold(minDistance, maxDistance, float(-choke), maxDistance+1.f, color, basis, bounds);
}

PlacedImagePtr EffectRenderer::drawDistanceThreshold(float minDistance, float maxDistance, float lowerThreshold, float upperThreshold, const Color &color, const PlacedImagePtr &basis, const ScaledBounds &outputBounds) {
    if (!(basis && outputBounds))
        return nullptr;
    DistanceTransformShader *distanceTransformShader = shaders.getDistanceTransformShader(basis->transparencyMode() == Image::RED_IS_ALPHA ? 'r' : 'a');
    DistanceThresholdShader *distanceThresholdShader = shaders.getDistanceThresholdShader();
    if (!(distanceTransformShader && distanceThresholdShader))
        return nullptr;

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds intermediateBounds = basis.bounds();
    intermediateBounds.a.x = outputBounds.a.x;
    intermediateBounds.b.x = outputBounds.b.x;
    PixelBounds pixelBounds = outerPixelBounds(intermediateBounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(pixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    distanceTransformShader->bind(pixelBounds, intermediateBounds, basis.bounds(), Vector2f(1.f, 0.f), minDistance, maxDistance);
    basisTex->bind(DistanceTransformShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(outputBounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    distanceThresholdShader->bind(pixelBounds, outputBounds, actualBounds, Vector2f(0.f, 1.f), minDistance, maxDistance, lowerThreshold, upperThreshold, color);
    intermediateTex->bind(DistanceThresholdShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    actualBounds = ScaledBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds);
}

}
