
#include "LinearDistanceTransformShader.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

namespace ode {

LinearDistanceTransformShader::LinearDistanceTransformShader() : precision(0) { }

bool LinearDistanceTransformShader::initialize(const SharedResource &res, char channel, int precision) {
    ODE_ASSERT(channel && precision > 0);
    if (!res)
        return false;
    char macros[256];
    sprintf(macros,
        "#define STEPS %d\n"
        "#define CHANNEL %c\n",
        precision, channel
    );
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D basis;"
        "uniform vec2 distanceFactor;"
        "uniform vec2 deltaStep;"
        "uniform float sdLow;"
        "uniform float invSdRange;"
        "void main() {"
            "bool inside =" ODE_GLSL_TEXTURE2D "(basis, texCoord).CHANNEL != 0.0;"
            ODE_GLSL_FRAGCOLOR ".ra = vec2(float(inside), 1.0);"
            "vec2 delta = vec2(0.0);"
            "for (int i = 0; i < STEPS; ++i) {"
                "delta += deltaStep;"
                "if ((" ODE_GLSL_TEXTURE2D "(basis, texCoord-delta).CHANNEL != 0.0) != inside || (" ODE_GLSL_TEXTURE2D "(basis, texCoord+delta).CHANNEL != 0.0) != inside) {"
                    ODE_GLSL_FRAGCOLOR ".r = invSdRange*((inside ? 1.0 : -1.0)*(dot(distanceFactor, delta)-0.5)-sdLow);"
                    "break;"
                "}"
            "}"
            ODE_GLSL_FRAGCOLOR ".gb =" ODE_GLSL_FRAGCOLOR ".rr;"
        "}\n"
    );
    FragmentShader fs("effect-lin-dist-transform");
    const GLchar *src[] = { ODE_EFFECT_SHADER_PREAMBLE, macros, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_EFFECT_SHADER_PREAMBLE)-1, (GLint) strlen(macros), fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    this->precision = precision;
    unifBasis = shader.getUniform("basis");
    unifDistanceFactor = shader.getUniform("distanceFactor");
    unifDeltaStep = shader.getUniform("deltaStep");
    unifSdLow = shader.getUniform("sdLow");
    unifInvSdRange = shader.getUniform("invSdRange");
    shader.bind();
    unifBasis.setInt(UNIT_BASIS);
    return EffectShader::initialize(&shader);
}

void LinearDistanceTransformShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, const Vector2f &direction, float minDistance, float maxDistance) {
    ODE_ASSERT(minDistance <= 0.f && maxDistance >= 0.f);
    shader.bind();
    EffectShader::bind(viewport, outputBounds, inputBounds);
    float distanceStep = std::max(-minDistance, maxDistance)/float(precision);
    float distanceFactor[2] = { float(inputBounds.dimensions().x), float(inputBounds.dimensions().y) };
    float deltaStep[2] = {
        distanceStep*direction.x/distanceFactor[0],
        distanceStep*direction.y/distanceFactor[1]
    };
    unifDistanceFactor.setVec2(distanceFactor);
    unifDeltaStep.setVec2(deltaStep);
    unifSdLow.setFloat(minDistance);
    unifInvSdRange.setFloat(1.f/(maxDistance-minDistance));
}

}
