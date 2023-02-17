#pragma once

#include <ode-graphics.h>
#include "CompositingShader.h"

namespace ode {

class AlphaMultShader : public CompositingShader {

public:
    static constexpr int UNIT_IN = 0;

    AlphaMultShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, float alphaFactor);

private:
    ShaderProgram shader;
    Uniform unifDstImage;
    Uniform unifAlphaFactor;

};

}
