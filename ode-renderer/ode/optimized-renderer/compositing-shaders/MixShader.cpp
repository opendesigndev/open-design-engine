
#include "MixShader.h"

namespace ode {

MixShader::MixShader() = default;

bool MixShader::initialize(const SharedResource& res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord[3];"
        "uniform sampler2D a;"
        "uniform sampler2D b;"
        "uniform float ratio;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "= mix(" ODE_GLSL_TEXTURE2D "(a, texCoord[0]), " ODE_GLSL_TEXTURE2D "(b, texCoord[1]), ratio);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-mix");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifAImage = shader.getUniform("a");
    unifBImage = shader.getUniform("b");
    unifRatio = shader.getUniform("ratio");
    shader.bind();
    unifAImage.setInt(UNIT_A);
    unifBImage.setInt(UNIT_B);
    return CompositingShader::initialize(&shader);
}

void MixShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &aBounds, const ScaledBounds &bBounds, float ratio) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds, aBounds, bBounds);
    unifRatio.setFloat(ratio);
}

}
