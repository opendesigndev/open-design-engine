
#include "BlitShader.h"

namespace ode {

BlitShader::BlitShader() = default;

bool BlitShader::initialize(const SharedResource &res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord[3];"
        "uniform sampler2D src;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "=" ODE_GLSL_TEXTURE2D "(src, texCoord[0]);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-blit");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifSrcImage = shader.getUniform("src");
    shader.bind();
    unifSrcImage.setInt(UNIT_IN);
    return CompositingShader::initialize(&shader);
}

void BlitShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds, inputBounds);
}

}
