
#pragma once

#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

namespace ode {

class SDFTextShader {

public:
    static constexpr int UNIT_IN = 0;

    SDFTextShader();
    SDFTextShader(const SDFTextShader &) = delete;
    SDFTextShader &operator=(const SDFTextShader &) = delete;
    bool initialize(bool multiChannel);
    bool ready() const;
    void bind(const PixelBounds &viewport, const Matrix3x3f &transformation);
    Uniform &texCoordFactorUniform();

private:
    ShaderProgram shader;
    Uniform unifTransformation;
    Uniform unifSDF;
    Uniform unifTexCoordFactor;
    Uniform unifRangeFactor;

};

}
