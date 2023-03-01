
#include "DesignEditorShader.h"

DesignEditorShader::SharedVertexShader::SharedVertexShader(const char *vsLabel) :
    VertexShader(vsLabel) {
}

bool DesignEditorShader::SharedVertexShader::initialize() {
    const std::string vertexShaderSource =
        "attribute vec2 coord;\n"
        "varying vec2 srcCoord;\n"
        "varying vec2 dstCoord;\n"
        "uniform mat2 framing;\n"
        "void main() {\n"
        "   gl_Position = vec4(framing[0]+framing[1]*coord, 0.0, 1.0);\n"
        "   srcCoord = coord;\n"
        "   dstCoord = 0.5+0.5*gl_Position.xy;\n"
        "}\n";

    const GLchar* src[] = { ODE_GLOBAL_SHADER_PREAMBLE, vertexShaderSource.c_str() };
    const GLint sln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, static_cast<GLint>(vertexShaderSource.size()) };

    return Shader::initialize(src, sln, 2);
}

DesignEditorShader::DesignEditorShader() {}

bool DesignEditorShader::initialize(const SharedVertexShader &sharedVertexShader, const char *fsLabel) {
    FragmentShader fs(fsLabel);
    if (!initializeFragmentShader(fs)) {
        return false;
    }

    if (!shader_.initialize(&sharedVertexShader, &fs)) {
        return false;
    }

    unifFraming_ = shader_.getUniform("framing");
    initializeUniforms();

    return true;
}

void DesignEditorShader::bind(const ScaledBounds& srcBounds, const ScaledBounds& dstBounds) {
    shader_.bind();
    const float framing[4] = {
        float(2.*(srcBounds.a.x-dstBounds.a.x)/dstBounds.dimensions().x-1.),
        float(2.*(srcBounds.a.y-dstBounds.a.y)/dstBounds.dimensions().y-1.),
        float(2.*srcBounds.dimensions().x/dstBounds.dimensions().x),
        float(2.*srcBounds.dimensions().y/dstBounds.dimensions().y),
    };
    unifFraming_.setMat2(framing);
}
