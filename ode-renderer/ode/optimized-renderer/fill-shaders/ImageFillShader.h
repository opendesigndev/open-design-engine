
#pragma once

#include "FillShader.h"

namespace ode {

class ImageFillShader : public FillShader {

public:
    static constexpr int UNIT_IMAGE = 0;

    ImageFillShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform);

private:
    ShaderProgram shader;
    Uniform unifImage;

};

}
