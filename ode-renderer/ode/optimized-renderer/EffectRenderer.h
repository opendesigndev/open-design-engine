
#pragma once

#include <octopus/octopus.h>
#include <ode-graphics.h>
#include "../image/Image.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "compositing-shaders/BlitShader.h"
#include "effect-shaders/effect-shaders.h"

#define EFFECT_SHADER_PRECISION 16

namespace ode {

class EffectRenderer {

public:
    EffectRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard, BlitShader &blitShader);
    PlacedImagePtr drawEffect(const octopus::Effect &effect, const PlacedImagePtr &basis, double scale);

private:
    class ShaderManager {
    public:
        ShaderManager();
        BoundedBlurShader *getBoundedBlurShader(char channel = '\0');
        GaussianBlurShader *getGaussianBlurShader();
        DistanceTransformShader *getDistanceTransformShader(char channel = 'a');
        DistanceThresholdShader *getDistanceThresholdShader();
    private:
        EffectShader::SharedResource shaderRes;
        BoundedBlurShader boundedBlurShaders[3];
        GaussianBlurShader gaussianBlurShader;
        DistanceTransformShader distanceTransformShaders[2];
        DistanceThresholdShader distanceThresholdShader;
    };

    GraphicsContext &gc;
    ShaderManager shaders;
    TextureFrameBufferManager &tfbManager;
    Mesh &billboard;
    BlitShader &blitShader;

    PlacedImagePtr drawStroke(const octopus::Stroke &stroke, const PlacedImagePtr &basis, double scale);
    PlacedImagePtr drawShadow(octopus::Effect::Type type, const octopus::Shadow &shadow, PlacedImagePtr basis, double scale);
    PlacedImagePtr drawBoundedBlur(double blur, const PlacedImagePtr &basis);
    PlacedImagePtr drawGaussianBlur(double blur, const PlacedImagePtr &basis);
    PlacedImagePtr drawChoke(double choke, const Color &color, const PlacedImagePtr &basis);
    PlacedImagePtr drawDistanceThreshold(float minDistance, float maxDistance, float lowerThreshold, float upperThreshold, const Color &color, const PlacedImagePtr &basis, const ScaledBounds &outputBounds);

};

}
