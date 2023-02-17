
#pragma once

#include <ode-graphics.h>
#include "EffectShader.h"

namespace ode {

class DistanceThresholdShader : public EffectShader {

public:
    DistanceThresholdShader();
    bool initialize(const SharedResource &res, int precision);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, const Vector2f &direction, float minDistance, float maxDistance, float lowerThreshold, float upperThreshold, const Color &color);

private:
    ShaderProgram shader;
    int precision;
    Uniform unifSdf;
    Uniform unifDistanceStep;
    Uniform unifDeltaStep;
    Uniform unifSdLow;
    Uniform unifSdRange;
    Uniform unifThreshold;
    Uniform unifColor;

};

}
