
#include "SimpleBlitShader.h"

#include <string>

SimpleBlitShader::SimpleBlitShader() { }

bool SimpleBlitShader::initialize(const RGIShader::SharedVertexShader &sharedVertexShader) {
    return RGIShader::initialize(sharedVertexShader, "diagnostics-blit-fs");
}

void SimpleBlitShader::bind(const ScaledBounds& srcBounds, const ScaledBounds& dstBounds, int srcUnit, bool ignoreAlpha) {
    RGIShader::bind(srcBounds, dstBounds);

    unifSrcImage_.setInt(srcUnit);
    unifIgnoreAlpha_.setBool(ignoreAlpha);
}

bool SimpleBlitShader::initializeFragmentShader(FragmentShader &fragmentShader) const {
    const std::string fragmentShaderSource =
        "varying vec2 srcCoord;\n"
        "uniform sampler2D srcImage;\n"
        "uniform bool ignoreAlpha;\n"
        "void main() {\n"
        "   gl_FragColor = texture2D(srcImage, srcCoord);\n"
        "   if (ignoreAlpha) {\n"
        "       gl_FragColor.a = 1.0;\n"
        "   }\n"
        "}\n";

    const GLchar* src[3] = { ODE_GLOBAL_SHADER_PREAMBLE, fragmentShaderSource.c_str() };
    GLint sln[3] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, static_cast<GLint>(fragmentShaderSource.size()) };
    int srcCount = 2;

    return fragmentShader.initialize(src, sln, srcCount);
}

void SimpleBlitShader::initializeUniforms() {
    unifSrcImage_ = shader_.getUniform("srcImage");
    unifIgnoreAlpha_ = shader_.getUniform("ignoreAlpha");
}
