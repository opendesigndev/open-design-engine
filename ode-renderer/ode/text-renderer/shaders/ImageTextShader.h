
#pragma once

#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

namespace ode {

class ImageTextShader {

public:
    static constexpr int UNIT_IN = 0;

    ImageTextShader();
    ImageTextShader(const ImageTextShader &) = delete;
    ImageTextShader &operator=(const ImageTextShader &) = delete;
    bool initialize();
    bool ready() const;
    void bind(const PixelBounds &viewport, const Matrix3x3f &transformation);
    Uniform &texCoordFactorUniform();

private:
    ShaderProgram shader;
    Uniform unifTransformation;
    Uniform unifImage;
    Uniform unifTexCoordFactor;

};

}
