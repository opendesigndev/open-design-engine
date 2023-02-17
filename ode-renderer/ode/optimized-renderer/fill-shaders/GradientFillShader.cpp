
#include "GradientFillShader.h"

namespace ode {

StringLiteral GradientFillShader::shapeFunctionSource(octopus::Gradient::Type gradientType) {
    #define INV_TAU "0.15915494309189533576888376337251" // 1/(2*PI)
    #define GRADIENT_SHAPE_EXPR(x) ODE_STRLIT("float gradientShape(vec2 pos) { return " x "; }")
    switch (gradientType) {
        case octopus::Gradient::Type::LINEAR:
            return GRADIENT_SHAPE_EXPR("pos.x");
        case octopus::Gradient::Type::RADIAL:
            return GRADIENT_SHAPE_EXPR("length(pos)");
        case octopus::Gradient::Type::ANGULAR:
            return GRADIENT_SHAPE_EXPR("fract(" INV_TAU "*atan(pos.y, pos.x))");
        case octopus::Gradient::Type::DIAMOND:
            return GRADIENT_SHAPE_EXPR("abs(pos.x)+abs(pos.y)");
    }
    return StringLiteral();
}

GradientFillShader::GradientFillShader() : FillShader() { }

bool GradientFillShader::initialize(const SharedResource &res, const StringLiteral &shapeFunction) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D gradient;"
        "uniform vec2 texTransform;"
        "void main() {"
            "float gx = gradientShape(texCoord);"
            ODE_GLSL_FRAGCOLOR "=" ODE_GLSL_TEXTURE2D "(gradient, vec2(texTransform[0]*gx+texTransform[1], 0.0));"
            ODE_GLSL_FRAGCOLOR ".rgb *=" ODE_GLSL_FRAGCOLOR ".a;"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("fill-gradient");
    const GLchar *src[] = { ODE_FILL_SHADER_PREAMBLE, shapeFunction.string, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_FILL_SHADER_PREAMBLE)-1, shapeFunction.length, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifGradient = shader.getUniform("gradient");
    unifTexTransform = shader.getUniform("texTransform");
    shader.bind();
    unifGradient.setInt(UNIT_GRADIENT);
    return FillShader::initialize(&shader);
}

void GradientFillShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform, const Vector2f &texTransform) {
    float tt[2] = { texTransform.x, texTransform.y };
    shader.bind();
    FillShader::bind(viewport, outputBounds, fragTransform);
    unifTexTransform.setVec2(tt);
}

}
