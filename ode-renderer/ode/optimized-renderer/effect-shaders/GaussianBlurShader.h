
#pragma once

#include <ode-graphics.h>
#include "EffectShader.h"

namespace ode {

class GaussianBlurShader : public EffectShader {

public:
    GaussianBlurShader();
    bool initialize(const SharedResource &res, char channel, int precision);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, bool phase, double sigma, const Color &color);

private:
    ShaderProgram shader;
    int precision;
    Uniform unifBasis;
    Uniform unifStepFactor;
    Uniform unifColor;

};

}
