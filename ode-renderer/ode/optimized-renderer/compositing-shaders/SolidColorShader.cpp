
#include "SolidColorShader.h"

namespace ode {

SolidColorShader::SolidColorShader() = default;

bool SolidColorShader::initialize(const SharedResource &res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        "uniform vec4 color;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "= color;"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-solid-color");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifColor = shader.getUniform("color");
    return CompositingShader::initialize(&shader);
}

void SolidColorShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Color &color) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds);
    float premColor[4] = {
        float(color.r*color.a),
        float(color.g*color.a),
        float(color.b*color.a),
        float(color.a),
    };
    unifColor.setVec4(premColor);
}

}
