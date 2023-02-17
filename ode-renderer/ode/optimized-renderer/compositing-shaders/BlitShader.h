
#pragma once

#include "CompositingShader.h"

namespace ode {

class BlitShader : public CompositingShader {

public:
    static constexpr int UNIT_IN = 0;

    BlitShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds);

private:
    ShaderProgram shader;
    Uniform unifSrcImage;

};

}
