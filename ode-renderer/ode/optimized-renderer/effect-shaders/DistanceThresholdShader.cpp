
#include "DistanceThresholdShader.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

namespace ode {

DistanceThresholdShader::DistanceThresholdShader() : precision(0) { }

bool DistanceThresholdShader::initialize(const SharedResource &res, int precision) {
    ODE_ASSERT(precision > 0);
    if (!res)
        return false;
    char stepsDefine[64];
    snprintf(stepsDefine, sizeof(stepsDefine), "#define STEPS %d\n", precision);
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D sdf;"
        "uniform float distanceStep;"
        "uniform vec2 deltaStep;"
        "uniform float sdLow;"
        "uniform float sdRange;"
        "uniform vec2 threshold;"
        "uniform vec4 color;"
        "float signedDistance() {"
            "float linearSd = sdLow+sdRange*" ODE_GLSL_TEXTURE2D "(sdf, texCoord).r;"
            "bool inside = linearSd >= 0.0;"
            "float minSquaredDistance = linearSd*linearSd;"
            "float orthogonalDistance = -0.5;"
            "vec2 delta = vec2(0.0);"
            "for (int i = 0; i < STEPS; ++i) {"
                "orthogonalDistance += distanceStep;"
                "delta += deltaStep;"
                "float squaredOrthogonalDistance = orthogonalDistance*orthogonalDistance;"
                "float squaredDistance = squaredOrthogonalDistance;"
                "linearSd = sdLow+sdRange*" ODE_GLSL_TEXTURE2D "(sdf, texCoord-delta).r;"
                "if ((linearSd >= 0.0) == inside)"
                    "squaredDistance += linearSd*linearSd;"
                "minSquaredDistance = min(minSquaredDistance, squaredDistance);"
                "squaredDistance = squaredOrthogonalDistance;"
                "linearSd = sdLow+sdRange*" ODE_GLSL_TEXTURE2D "(sdf, texCoord+delta).r;"
                "if ((linearSd >= 0.0) == inside)"
                    "squaredDistance += linearSd*linearSd;"
                "minSquaredDistance = min(minSquaredDistance, squaredDistance);"
            "}"
            "return (inside ? 1.0 : -1.0)*sqrt(minSquaredDistance);"
        "}"
        "void main() {"
            "float sd = signedDistance();"
            "float opacity = clamp(sd-threshold[0]+0.5, 0.0, 1.0)*clamp(threshold[1]-sd+0.5, 0.0, 1.0);"
            ODE_GLSL_FRAGCOLOR "= opacity*color;"
        "}\n"
    );
    FragmentShader fs("effect-dist-threshold");
    const GLchar *src[] = { ODE_EFFECT_SHADER_PREAMBLE, stepsDefine, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_EFFECT_SHADER_PREAMBLE)-1, (GLint) strlen(stepsDefine), fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    this->precision = precision;
    unifSdf = shader.getUniform("sdf");
    unifDistanceStep = shader.getUniform("distanceStep");
    unifDeltaStep = shader.getUniform("deltaStep");
    unifSdLow = shader.getUniform("sdLow");
    unifSdRange = shader.getUniform("sdRange");
    unifThreshold = shader.getUniform("threshold");
    unifColor = shader.getUniform("color");
    shader.bind();
    unifSdf.setInt(UNIT_BASIS);
    return EffectShader::initialize(&shader);
}

void DistanceThresholdShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds, const Vector2f &direction, float minDistance, float maxDistance, float lowerThreshold, float upperThreshold, const Color &color) {
    ODE_ASSERT(minDistance <= 0.f && maxDistance >= 0.f);
    shader.bind();
    EffectShader::bind(viewport, outputBounds, inputBounds);
    float distanceStep = std::max(-minDistance, maxDistance)/float(precision);
    float distanceFactor[2] = { float(inputBounds.dimensions().x), float(inputBounds.dimensions().y) };
    float deltaStep[2] = {
        distanceStep*direction.x/distanceFactor[0],
        distanceStep*direction.y/distanceFactor[1]
    };
    float threshold[2] = { float(lowerThreshold), float(upperThreshold) };
    float premultipliedColor[4] = {
        float(color.a*color.r),
        float(color.a*color.g),
        float(color.a*color.b),
        float(color.a)
    };
    unifDistanceStep.setFloat(distanceStep);
    unifDeltaStep.setVec2(deltaStep);
    unifSdLow.setFloat(minDistance);
    unifSdRange.setFloat(maxDistance-minDistance);
    unifThreshold.setVec2(threshold);
    unifColor.setVec4(premultipliedColor);
}

}
