
#pragma once

#include <ode-graphics.h>
#include "EffectShader.h"

namespace ode {

class LinearBlurShader : public EffectShader {

public:
    LinearBlurShader();
    bool initialize(const SharedResource &res, bool alphaOnly, int precision);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, bool phase, float sigma, const Color &color);

private:
    ShaderProgram shader;
    int precision;
    Uniform unifBasis;
    Uniform unifStepFactor;
    Uniform unifColor;

};

}
