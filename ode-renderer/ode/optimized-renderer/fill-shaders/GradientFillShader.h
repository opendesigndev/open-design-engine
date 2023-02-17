
#pragma once

#include <octopus/octopus.h>
#include "FillShader.h"

namespace ode {

class GradientFillShader : public FillShader {

public:
    static constexpr int UNIT_GRADIENT = 0;

    static StringLiteral shapeFunctionSource(octopus::Gradient::Type gradientType);

    GradientFillShader();
    bool initialize(const SharedResource &res, const StringLiteral &shapeFunction);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform, const Vector2f &texTransform);

private:
    ShaderProgram shader;
    Uniform unifGradient;
    Uniform unifTexTransform;

};

}
