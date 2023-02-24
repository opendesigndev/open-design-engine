
#include "LinearBlurShader.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

namespace ode {

LinearBlurShader::LinearBlurShader() : precision(0) { }

bool LinearBlurShader::initialize(const SharedResource &res, char channel, int precision) {
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
        "float invErf(float x) {"
            "float l = log(1.0-x*x);"
            "float g = 4.546884979448284327344753864428+0.5*l;"
            "return sqrt(sqrt(g*g-7.1422302240762540265936395279122*l)-g);"
        "}"
        "void main() {"
            "SUM_TYPE sum = " ODE_GLSL_TEXTURE2D "(basis, texCoord) CHANNELS;"
            "for (int i = 2; i <= STEPS; i += 2) {"
                "vec2 offset = invErf(STEP_WEIGHT*float(i))*stepFactor;"
                "sum += " ODE_GLSL_TEXTURE2D "(basis, texCoord-offset) CHANNELS;"
                "sum += " ODE_GLSL_TEXTURE2D "(basis, texCoord+offset) CHANNELS;"
            "}"
            ODE_GLSL_FRAGCOLOR "= STEP_WEIGHT*sum*color;"
        "}\n"
    );
    FragmentShader fs("effect-lin-blur");
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

void LinearBlurShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, bool phase, float sigma, const Color &color) {
    ODE_ASSERT(sigma > 0.f);
    shader.bind();
    EffectShader::bind(viewport, outputBounds, inputBounds);
    float stepFactor[2] = {
        sigma/float(inputBounds.dimensions().x),
        sigma/float(inputBounds.dimensions().y)
    };
    if (phase)
        stepFactor[0] = -stepFactor[0];
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
