
#include "BlendShader.h"

#include <cstring>

namespace ode {

StringLiteral BlendShader::blendFunctionSource(octopus::BlendMode blendMode) {
    #define LUM_FN \
        "float getLum(vec3 c) {" \
            "return dot(c, vec3(0.3, 0.59, 0.11));" \
        "}" \
        "vec3 clipColor(vec3 c) {" \
            "float l = getLum(c);" \
            "float n = min(c.r, min(c.g, c.b));" \
            "float x = max(c.r, max(c.g, c.b));" \
            "c = n < 0.0 ? l + (c - l) * l / (l - n) : c;" \
            "c = x > 1.0 ? l + (c - l) * (1.0 - l) / (x - l) : c;" \
            "return c;" \
        "}" \
        "vec3 setLum(vec3 c, float l) {" \
            "return clipColor(c + l - getLum(c));" \
        "}"

    #define SAT_FN \
        "float getSat(vec3 c) {" \
            "return max(c.r, max(c.g, c.b)) - min(c.r, min(c.g, c.b));" \
        "}" \
        "vec3 setSat(vec3 c, float s) {" \
            "float cMax = max(c.r, max(c.g, c.b));" \
            "float cMin = min(c.r, min(c.g, c.b));" \
            "float cMid = max(min(c.r, c.g), min(max(c.r, c.g), c.b));" \
            "float newMid = ((cMid - cMin) * s) / (cMax - cMin + 0.000001);" \
            "c.r = mix(s, mix(newMid, 0.0, step(c.r, cMin)), step(c.r, cMid));" \
            "c.g = mix(s, mix(newMid, 0.0, step(c.g, cMin)), step(c.g, cMid));" \
            "c.b = mix(s, mix(newMid, 0.0, step(c.b, cMin)), step(c.b, cMid));" \
            "return c;" \
        "}"

    #define RGB_WEIGHT "vec3(0.299, 0.587, 0.114)"

    #define BLEND_COLOR_BODY(x) ODE_STRLIT("vec3 blendColor(vec4 dst, vec4 src) {" x "}")
    #define BLEND_FUNC(x) ODE_STRLIT(x \
        "vec3 blendColor(vec4 dst, vec4 src) {" \
            "dst.rgb /= max(dst.a, 0.001);" \
            "src.rgb /= max(src.a, 0.001);" \
            "return mix(dst.rgb, mix(src.rgb, blendFunc(dst.rgb, src.rgb), dst.a), src.a/max(" ODE_GLSL_FRAGCOLOR ".a, 0.001))*" ODE_GLSL_FRAGCOLOR ".a;" \
        "}" \
    )
    #define BLEND_FUNC_BODY(x) BLEND_FUNC("vec3 blendFunc(vec3 dst, vec3 src) {" x "}")

    switch (blendMode) {
        // Functions optimized for alpha-premultiplied color space
        case octopus::BlendMode::NORMAL:
            return BLEND_COLOR_BODY("return dst.rgb-dst.rgb*src.a+src.rgb;");
        case octopus::BlendMode::MULTIPLY:
            return BLEND_COLOR_BODY("return dst.rgb+src.rgb-dst.rgb*src.a-src.rgb*dst.a+dst.rgb*src.rgb;");
        case octopus::BlendMode::SCREEN: // (inverse multiplicative)
            return BLEND_COLOR_BODY("return dst.rgb+src.rgb-dst.rgb*src.rgb;");
        case octopus::BlendMode::LINEAR_DODGE: // (additive)
            return BLEND_COLOR_BODY("return dst.rgb+src.rgb;");
        case octopus::BlendMode::LINEAR_BURN: // (inverse additive / subtractive)
            return BLEND_COLOR_BODY("return dst.rgb+src.rgb-dst.a*src.a;");
        // Functions done in unpremultiplied color space (some may possibly be converted to optimized versions)
        case octopus::BlendMode::COLOR_DODGE:
            return BLEND_FUNC_BODY("return mix(vec3(1.0), clamp(dst/(vec3(1.0)-src), vec3(0.0), vec3(1.0)), step(src, vec3(1.0)));");
        case octopus::BlendMode::COLOR_BURN:
            return BLEND_FUNC_BODY("return (vec3(1.0)-clamp((vec3(1.0)-dst)/src, vec3(0.0), vec3(1.0)))*step(vec3(0.0), src);");
        case octopus::BlendMode::SUBTRACT:
            return BLEND_FUNC_BODY("return max(dst-src, vec3(0.0));");
        case octopus::BlendMode::DIFFERENCE:
            return BLEND_FUNC_BODY("return abs(dst-src);");
        case octopus::BlendMode::EXCLUSION:
            return BLEND_FUNC_BODY("return dst-2.0*dst*src+src;");
        case octopus::BlendMode::DIVIDE:
            return BLEND_FUNC_BODY("return clamp(dst/src, vec3(0.0), vec3(1.0));");
        case octopus::BlendMode::DARKEN:
            return BLEND_FUNC_BODY("return min(dst, src);");
        case octopus::BlendMode::LIGHTEN:
            return BLEND_FUNC_BODY("return max(dst, src);");
        case octopus::BlendMode::DARKER_COLOR:
            return BLEND_FUNC_BODY("return dot(dst," RGB_WEIGHT ") < dot(src," RGB_WEIGHT ") ? dst : src;");
        case octopus::BlendMode::LIGHTER_COLOR:
            return BLEND_FUNC_BODY("return dot(dst," RGB_WEIGHT ") > dot(src," RGB_WEIGHT ") ? dst : src;");
        case octopus::BlendMode::OVERLAY:
            return BLEND_FUNC_BODY("return mix(2.0*dst*src, 1.0-2.0*(1.0-dst)*(1.0-src), step(vec3(0.5), dst));");
        case octopus::BlendMode::SOFT_LIGHT:
            return BLEND_FUNC_BODY("return mix(2.0*dst*src+dst*dst*(1.0-2.0*src), sqrt(dst)*(2.0*src-1.0)+2.0*dst*(1.0-src), step(0.5, src));");
        case octopus::BlendMode::HARD_LIGHT:
            return BLEND_FUNC_BODY("return mix(2.0*src*dst, 1.0-2.0*(1.0-src)*(1.0-dst), step(vec3(0.5), src));");
        case octopus::BlendMode::VIVID_LIGHT:
            return BLEND_FUNC_BODY("return clamp(mix(mix(src, vec3(1.0)-0.5*(vec3(1.0)-dst)/src, step(vec3(0.0), src)), min(vec3(1.0), mix(vec3(1.0), 0.5*dst/(vec3(1.0)-src), step(src, vec3(1.0)))), step(vec3(0.5), src)), vec3(0.0), vec3(1.0));");
        case octopus::BlendMode::LINEAR_LIGHT:
            return BLEND_FUNC_BODY("return clamp(mix(max(dst+2.0*src-vec3(1.0), vec3(0.0)), 2.0*src+dst-vec3(1.0), step(vec3(0.5), dst)), vec3(0.0), vec3(1.0));");
        case octopus::BlendMode::PIN_LIGHT:
            return BLEND_FUNC_BODY("return mix(min(dst, 2.0*src), max(dst, 2.0*src-vec3(1.0)), step(vec3(0.5), src));");
        case octopus::BlendMode::HARD_MIX:
            return BLEND_FUNC_BODY("return step(vec3(1.0), dst+src);");
        case octopus::BlendMode::HUE:
            return BLEND_FUNC(LUM_FN SAT_FN "vec3 blendFunc(vec3 dst, vec3 src) { return setLum(setSat(src, getSat(dst)), getLum(dst)); }");
        case octopus::BlendMode::SATURATION:
            return BLEND_FUNC(LUM_FN SAT_FN "vec3 blendFunc(vec3 dst, vec3 src) { return setLum(setSat(dst, getSat(src)), getLum(dst)); }");
        case octopus::BlendMode::COLOR:
            return BLEND_FUNC(LUM_FN "vec3 blendFunc(vec3 dst, vec3 src) { return setLum(src, getLum(dst)); }");
        case octopus::BlendMode::LUMINOSITY:
            return BLEND_FUNC(LUM_FN "vec3 blendFunc(vec3 dst, vec3 src) { return setLum(dst, getLum(src)); }");
        case octopus::BlendMode::PASS_THROUGH:
            break;
    }
    return StringLiteral();
}

BlendShader::BlendShader() = default;

bool BlendShader::initialize(const SharedResource &res, const StringLiteral &blendFunction) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord[3];"
        "uniform sampler2D dst;"
        "uniform sampler2D src;"
        "uniform bool ignoreSrcAlpha;"
        "void main() {"
            "vec4 d = " ODE_GLSL_TEXTURE2D "(dst, texCoord[0]);"
            "vec4 s = " ODE_GLSL_TEXTURE2D "(src, texCoord[1]);"
            "if (ignoreSrcAlpha) {"
                "s.rgb /= max(s.a, 0.001);"
                "s.a = 1.0;"
            "}"
            ODE_GLSL_FRAGCOLOR ".a = d.a-d.a*s.a+s.a;"
            ODE_GLSL_FRAGCOLOR ".rgb = blendColor(d, s);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("compositing-blend");
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, blendFunction.string, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, blendFunction.length, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifDstImage = shader.getUniform("dst");
    unifSrcImage = shader.getUniform("src");
    unifIgnoreSrcAlpha = shader.getUniform("ignoreSrcAlpha");
    shader.bind();
    unifDstImage.setInt(UNIT_DST);
    unifSrcImage.setInt(UNIT_SRC);
    return CompositingShader::initialize(&shader);
}

void BlendShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &dstBounds, const ScaledBounds &srcBounds, bool ignoreSrcAlpha) {
    shader.bind();
    CompositingShader::bind(viewport, outputBounds, dstBounds, srcBounds);
    unifIgnoreSrcAlpha.setBool(ignoreSrcAlpha);
}

}
