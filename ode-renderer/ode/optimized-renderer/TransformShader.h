
#pragma once

#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

namespace ode {

// TEMPORARY / LEGACY - TODO DELETE THIS
class TransformShader {

public:
    static constexpr int UNIT_IN = 0;

    TransformShader();
    TransformShader(const TransformShader &) = delete;
    TransformShader &operator=(const TransformShader &) = delete;
    bool initialize();
    bool ready() const;
    void bind(const Matrix3x3f &transformation, const Vector2i &outputSize);

private:
    ShaderProgram shader;
    Uniform unifTransformation;
    Uniform unifImage;

};

}
