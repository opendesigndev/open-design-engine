
#include "CanvasShader.h"

#include <string>

CanvasShader::CanvasShader() { }

bool CanvasShader::initialize(const DesignEditorShader::SharedVertexShader &sharedVertexShader) {
    return DesignEditorShader::initialize(sharedVertexShader, "design-editor-fs");
}

void CanvasShader::bind(const ScaledBounds& srcBounds,
                        const ScaledBounds& dstBounds,
                        int srcUnit,
                        const Vector2i &resolution,
                        const SelectionRectangleOpt &selectionRectangle,
                        bool ignoreAlpha) {
    DesignEditorShader::bind(srcBounds, dstBounds);

    const float res[2] = {
        float(resolution.x),
        float(resolution.y)
    };
    const float selectionRect[4] = {
        selectionRectangle.has_value() ? selectionRectangle->a.x : 0,
        selectionRectangle.has_value() ? selectionRectangle->a.y : 0,
        selectionRectangle.has_value() ? selectionRectangle->b.x : 0,
        selectionRectangle.has_value() ? selectionRectangle->b.y : 0,
    };

    unifSrcImage_.setInt(srcUnit);
    unifResolution_.setVec2(res);
    unifIgnoreAlpha_.setBool(ignoreAlpha);
    unifSelectionRectangle_.setMat2(selectionRect);
}

bool CanvasShader::initializeFragmentShader(FragmentShader &fragmentShader) const {
    const std::string fragmentShaderSource =
        "varying vec2 srcCoord;\n"
        "uniform sampler2D srcImage;\n"
        "uniform vec2 resolution;\n"
        "uniform bool ignoreAlpha;\n"
        "uniform mat2 selectionRectangle;\n"
        "float lineSegment(vec2 p, vec2 a, vec2 b) {\n"
        "    vec2 pa = p - a, ba = b - a;\n"
        "    float h = clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0);\n"
        "    return smoothstep(0.0, 1.0 / resolution.x, length(pa - ba*h));\n"
        "}\n"
        "void main() {\n"
        "    vec2 st = gl_FragCoord.xy/resolution.xy;\n"
        "    float l = selectionRectangle[0][0];\n"
        "    float r = selectionRectangle[1][0];\n"
        "    float t = selectionRectangle[0][1];\n"
        "    float b = selectionRectangle[1][1];\n"
        "    vec3 rect = vec3(lineSegment(st, vec2(l,t), vec2(r,t)));\n"
        "    rect = mix(vec3(0.5), rect, lineSegment(st, vec2(l,t), vec2(l,b)));\n"
        "    rect = mix(vec3(0.5), rect, lineSegment(st, vec2(r,t), vec2(r,b)));\n"
        "    rect = mix(vec3(0.5), rect, lineSegment(st, vec2(l,b), vec2(r,b)));\n"
        "    gl_FragColor = min(vec4(rect, 1.0), texture2D(srcImage, srcCoord));\n"
        "    if (ignoreAlpha) {\n"
        "        gl_FragColor.a = 1.0;\n"
        "    }\n"
        "}\n";

    const GLchar* src[3] = { ODE_GLOBAL_SHADER_PREAMBLE, fragmentShaderSource.c_str() };
    GLint sln[3] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, static_cast<GLint>(fragmentShaderSource.size()) };
    int srcCount = 2;

    return fragmentShader.initialize(src, sln, srcCount);
}

void CanvasShader::initializeUniforms() {
    unifSrcImage_ = shader_.getUniform("srcImage");
    unifResolution_ = shader_.getUniform("resolution");
    unifIgnoreAlpha_ = shader_.getUniform("ignoreAlpha");
    unifSelectionRectangle_ = shader_.getUniform("selectionRectangle");
}
