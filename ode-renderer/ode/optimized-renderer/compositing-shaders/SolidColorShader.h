
#pragma once

#include "CompositingShader.h"

namespace ode {

class SolidColorShader : public CompositingShader {

public:
    SolidColorShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Color &color);

private:
    ShaderProgram shader;
    Uniform unifColor;

};

}
