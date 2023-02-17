
#pragma once

#include "CompositingShader.h"

namespace ode {

class MixShader : public CompositingShader {

public:
    static constexpr int UNIT_A = 0;
    static constexpr int UNIT_B = 1;

    MixShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &aBounds, const ScaledBounds &bBounds, float ratio);

private:
    ShaderProgram shader;
    Uniform unifAImage;
    Uniform unifBImage;
    Uniform unifRatio;

};

}
