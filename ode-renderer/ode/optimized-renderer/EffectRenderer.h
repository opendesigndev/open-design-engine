
#pragma once

#include <octopus/octopus.h>
#include <ode-graphics.h>
#include "../image/Image.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "compositing-shaders/BlitShader.h"
#include "effect-shaders/effect-shaders.h"

#define EFFECT_SHADER_PRECISION 16
#define BLUR_MARGIN_FACTOR 2.8856349124267571473876066463246112449484013782094917195589559

namespace ode {

class EffectRenderer {

public:
    EffectRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard, BlitShader &blitShader);
    PlacedImagePtr drawEffect(const octopus::Effect &effect, const PlacedImagePtr &basis, double scale);

private:
    GraphicsContext &gc;
    TextureFrameBufferManager &tfbManager;
    Mesh &billboard;
    BlitShader &blitShader;

    EffectShader::SharedResource shaderRes;
    LinearBlurShader blurShaders[3];
    LinearDistanceTransformShader distanceTransformShaders[2];
    DistanceThresholdShader distanceThresholdShader;

    LinearBlurShader *requireBlurShader(char channel = '\0');
    LinearDistanceTransformShader *requireDistanceTransformShader(char channel = 'a');
    DistanceThresholdShader *requireDistanceThresholdShader();

    PlacedImagePtr drawStroke(const octopus::Stroke &stroke, const PlacedImagePtr &basis, double scale);
    PlacedImagePtr drawShadow(octopus::Effect::Type type, const octopus::Shadow &shadow, PlacedImagePtr basis, double scale);
    PlacedImagePtr drawBlur(double blur, const PlacedImagePtr &basis);
    PlacedImagePtr drawChoke(double choke, const Color &color, const PlacedImagePtr &basis);
    PlacedImagePtr drawDistanceThreshold(float minDistance, float maxDistance, float lowerThreshold, float upperThreshold, const Color &color, const PlacedImagePtr &basis, const ScaledBounds &outputBounds);

};

}
