
#include "MixMaskShader.h"

namespace ode {

MixMaskShader::MixMaskShader() = default;

bool MixMaskShader::initialize(const SharedResource& res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord[3];"
        "uniform sampler2D a;"
        "uniform sampler2D b;"
        "uniform sampler2D mask;"
        "uniform vec4 maskChannelFactor;"
        "uniform float maskBias;"
        "void main() {"
            "vec4 m = " ODE_GLSL_TEXTURE2D "(mask, texCoord[2]);"
            "float ratio = dot(maskChannelFactor.rgb, m.rgb)/max(m.a, 0.001) + maskChannelFactor.a*m.a + maskBias;"
            ODE_GLSL_FRAGCOLOR "= mix(" ODE_GLSL_TEXTURE2D "(a, texCoord[0]), " ODE_GLSL_TEXTURE2D "(b, texCoord[1]), ratio);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-mix-mask");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifAImage = shader.getUniform("a");
    unifBImage = shader.getUniform("b");
    unifMaskImage = shader.getUniform("mask");
    unifMaskChannelFactor = shader.getUniform("maskChannelFactor");
    unifMaskBias = shader.getUniform("maskBias");
    shader.bind();
    unifAImage.setInt(UNIT_A);
    unifBImage.setInt(UNIT_B);
    unifMaskImage.setInt(UNIT_MASK);
    return CompositingShader::initialize(&shader);
}

void MixMaskShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &aBounds, const ScaledBounds &bBounds, const ScaledBounds &maskBounds, const ChannelMatrix &channelMatrix) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds, aBounds, bBounds, maskBounds);
    float maskChannelFactor[4] = {
        float(channelMatrix.m[0]),
        float(channelMatrix.m[1]),
        float(channelMatrix.m[2]),
        float(channelMatrix.m[3]),
    };
    unifMaskChannelFactor.setVec4(maskChannelFactor);
    unifMaskBias.setFloat(float(channelMatrix.m[4]));
}

}
