
#include "AlphaMultShader.h"

namespace ode {

AlphaMultShader::AlphaMultShader() = default;

bool AlphaMultShader::initialize(const SharedResource &res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord[3];"
        "uniform sampler2D dst;"
        "uniform float alphaFactor;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "= alphaFactor*" ODE_GLSL_TEXTURE2D "(dst, texCoord[0]);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-alpha-mult");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifDstImage = shader.getUniform("dst");
    unifAlphaFactor = shader.getUniform("alphaFactor");
    shader.bind();
    unifDstImage.setInt(UNIT_IN);
    return CompositingShader::initialize(&shader);
}

void AlphaMultShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, float alphaFactor) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds, inputBounds);
    unifAlphaFactor.setFloat(alphaFactor);
}

}
