
#pragma once

#include <ode-graphics.h>
#include "EffectShader.h"

namespace ode {

class BoundedBlurShader : public EffectShader {

public:
    BoundedBlurShader();
    bool initialize(const SharedResource &res, char channel, int precision);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, bool phase, double radius, const Color &color);

private:
    ShaderProgram shader;
    int precision;
    Uniform unifBasis;
    Uniform unifStepFactor;
    Uniform unifColor;

};

}
