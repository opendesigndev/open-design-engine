
#pragma once

#include <ode-graphics.h>
#include "EffectShader.h"

namespace ode {

class LinearDistanceTransformShader : public EffectShader {

public:
    LinearDistanceTransformShader();
    bool initialize(const SharedResource &res, int precision);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, const Vector2f &direction, float minDistance, float maxDistance);

private:
    ShaderProgram shader;
    int precision;
    Uniform unifBasis;
    Uniform unifDistanceFactor;
    Uniform unifDeltaStep;
    Uniform unifSdLow;
    Uniform unifInvSdRange;

};

}
