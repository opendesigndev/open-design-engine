
#include "BoundedBlurShader.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

namespace ode {

BoundedBlurShader::BoundedBlurShader() : precision(0) { }

bool BoundedBlurShader::initialize(const SharedResource &res, char channel, int precision) {
    ODE_ASSERT(precision > 0);
    if (!res)
        return false;
    char channelDef[] = " .?";
    channelDef[2] = channel;
    char macros[256];
    sprintf(macros,
        "#define STEPS %d\n"
        "#define SUM_TYPE %s\n"
        "#define CHANNELS%s\n",
        2*precision, channel ? "float" : "vec4", channel ? channelDef : ""
    );
    const StringLiteral fsSrc = ODE_STRLIT(
        "const float STEP_WEIGHT = 1.0/float(STEPS+1);"
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D basis;"
        "uniform vec2 stepFactor;"
        "uniform vec4 color;"
        "void main() {"
            "SUM_TYPE sum = " ODE_GLSL_TEXTURE2D "(basis, texCoord) CHANNELS;"
            "for (int i = 2; i <= STEPS; i += 2) {"
                "vec2 offset = (1.0-sqrt(STEP_WEIGHT*float(i)))*stepFactor;"
                "sum += " ODE_GLSL_TEXTURE2D "(basis, texCoord-offset) CHANNELS;"
                "sum += " ODE_GLSL_TEXTURE2D "(basis, texCoord+offset) CHANNELS;"
            "}"
            ODE_GLSL_FRAGCOLOR "= STEP_WEIGHT*sum*color;"
        "}\n"
    );
    FragmentShader fs("effect-bound-blur");
    const GLchar *src[] = { ODE_EFFECT_SHADER_PREAMBLE, macros, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_EFFECT_SHADER_PREAMBLE)-1, (GLint) strlen(macros), fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    this->precision = precision;
    unifBasis = shader.getUniform("basis");
    unifStepFactor = shader.getUniform("stepFactor");
    unifColor = shader.getUniform("color");
    shader.bind();
    unifBasis.setInt(UNIT_BASIS);
    return EffectShader::initialize(&shader);
}

void BoundedBlurShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, bool phase, double radius, const Color &color) {
    shader.bind();
    EffectShader::bind(viewport, outputBounds, inputBounds);
    float stepFactor[2] = { };
    if (!phase)
        stepFactor[0] = float(radius/inputBounds.dimensions().x);
    else
        stepFactor[1] = float(radius/inputBounds.dimensions().y);
    float premultipliedColor[4] = {
        float(color.a*color.r),
        float(color.a*color.g),
        float(color.a*color.b),
        float(color.a)
    };
    unifStepFactor.setVec2(stepFactor);
    unifColor.setVec4(premultipliedColor);
}

}
