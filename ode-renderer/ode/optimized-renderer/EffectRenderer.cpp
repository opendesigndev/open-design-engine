
#include "EffectRenderer.h"

#include <algorithm>

namespace ode {

EffectRenderer::EffectRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard, BlitShader &blitShader) : gc(gc), tfbManager(tfbManager), billboard(billboard), blitShader(blitShader), shaderRes(EffectShader::prepare()) { }

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
                return drawBlur(scale*effect.blur.value(), basis);
            return nullptr;
        case octopus::Effect::Type::OTHER:
            return nullptr;
    }
    ODE_ASSERT(!"Incomplete switch");
    return nullptr;
}

bool EffectRenderer::requireBlurShader() {
    return blurShader.ready() || blurShader.initialize(shaderRes, false, EFFECT_SHADER_PRECISION);
}

bool EffectRenderer::requireAlphaBlurShader() {
    return alphaBlurShader.ready() || alphaBlurShader.initialize(shaderRes, true, EFFECT_SHADER_PRECISION);
}

bool EffectRenderer::requireDistanceTransformShader() {
    return distanceTransformShader.ready() || distanceTransformShader.initialize(shaderRes, EFFECT_SHADER_PRECISION);
}

bool EffectRenderer::requireDistanceThresholdShader() {
    return distanceThresholdShader.ready() || distanceThresholdShader.initialize(shaderRes, EFFECT_SHADER_PRECISION);
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
    Vector2d offset = scale*fromOctopus(shadow.offset);
    double sigma = fabs(scale*shadow.blur);
    if (!sigma) {
        if (shadow.choke) {
            PlacedImagePtr choke = drawChoke(scale*shadow.choke, fromOctopus(shadow.color), basis);
            return PlacedImagePtr(choke, choke.bounds()+offset);
        }

        // Basis to shadow color
        TexturePtr basisTex = basis->asTexture();
        if (!basisTex)
            return nullptr;
        PixelBounds pixelBounds = outerPixelBounds(basis.bounds());
        TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
        ScaledBounds bounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion
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
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        blitShader.bind(pixelBounds, bounds, basis.bounds());
        basisTex->bind(BlitShader::UNIT_IN);
        billboard.draw();
        glDisable(GL_BLEND);
        outTex->unbind();
        return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), bounds+offset);
    }

    if (!requireAlphaBlurShader())
        return nullptr;
    if (shadow.choke)
        basis = drawChoke(scale*shadow.choke, Color(1), basis);

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds bounds = basis.bounds()+ScaledMargin(BLUR_MARGIN_FACTOR*sigma);
    PixelBounds pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(pixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    alphaBlurShader.bind(pixelBounds, bounds, basis.bounds(), false, float(sigma), Color(1));
    basisTex->bind(LinearBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    alphaBlurShader.bind(pixelBounds, bounds, actualBounds, true, float(sigma), fromOctopus(shadow.color));
    intermediateTex->bind(LinearBlurShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    actualBounds = ScaledBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds+offset);
}

PlacedImagePtr EffectRenderer::drawBlur(double blur, const PlacedImagePtr &basis) {
    if (!blur)
        return basis;
    if (!(basis && requireBlurShader()))
        return nullptr;

    TexturePtr basisTex = basis->asTexture();
    if (!basisTex)
        return nullptr;
    ScaledBounds bounds = basis.bounds()+ScaledMargin(BLUR_MARGIN_FACTOR*blur);
    PixelBounds pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr intermediateTex = tfbManager.acquire(pixelBounds);
    intermediateTex->bind();
    glViewport(0, 0, intermediateTex->dimensions().x, intermediateTex->dimensions().y);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader.bind(pixelBounds, bounds, basis.bounds(), false, float(blur), Color(1));
    basisTex->bind(LinearBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(bounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader.bind(pixelBounds, bounds, actualBounds, true, float(blur), Color(1));
    intermediateTex->bind(LinearBlurShader::UNIT_BASIS);
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
    if (!(basis && outputBounds && requireDistanceTransformShader() && requireDistanceThresholdShader()))
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
    distanceTransformShader.bind(pixelBounds, intermediateBounds, basis.bounds(), Vector2f(1.f, 0.f), minDistance, maxDistance);
    basisTex->bind(LinearBlurShader::UNIT_BASIS);
    billboard.draw();
    intermediateTex->unbind();
    ScaledBounds actualBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    pixelBounds = outerPixelBounds(outputBounds);
    TextureFrameBufferPtr outTex = tfbManager.acquire(pixelBounds);
    outTex->bind();
    glViewport(0, 0, outTex->dimensions().x, outTex->dimensions().y);
    glClear(GL_COLOR_BUFFER_BIT);
    distanceThresholdShader.bind(pixelBounds, outputBounds, actualBounds, Vector2f(0.f, 1.f), minDistance, maxDistance, lowerThreshold, upperThreshold, color);
    intermediateTex->bind(LinearBlurShader::UNIT_BASIS);
    billboard.draw();
    outTex->unbind();
    actualBounds = ScaledBounds(Vector2d(pixelBounds.a), Vector2d(pixelBounds.b)); // TODO proper conversion

    return PlacedImagePtr(Image::fromTexture(outTex, Image::PREMULTIPLIED), actualBounds);
}

}
