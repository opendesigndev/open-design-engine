
#include "ShaderProgram.h"

namespace ode {

ShaderProgram::ShaderProgram() : handle(0), ready(false) { }

ShaderProgram::~ShaderProgram() {
    if (handle) {
        glUseProgram(0); // in case it is still bound
        glDeleteProgram(handle);
        ODE_CHECK_GL_ERROR();
    }
    //LOG_OWN_ACTION(SHADER_DELETION, 1, "");
}

bool ShaderProgram::initialize(const VertexShader *vertexShader, const FragmentShader *fragmentShader) {
    if (ready || !(vertexShader && fragmentShader && *vertexShader && *fragmentShader))
        return false;
    if (!handle)
        handle = glCreateProgram();
    if (!handle)
        return false;
    glAttachShader(handle, vertexShader->handle);
    glAttachShader(handle, fragmentShader->handle);
    glLinkProgram(handle);
    ODE_CHECK_GL_ERROR();
    GLint status = GL_FALSE;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
#ifdef ODE_DEBUG
    ready = status == GL_TRUE;
#else
    if (!(ready = status == GL_TRUE))
#endif
    {
        GLint infoLogLength = -1;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0) {
            log.resize(infoLogLength);
            glGetProgramInfoLog(handle, infoLogLength, nullptr, &log[0]);
            //Log::instance.log(Log::CORE_OPENGL, Log::ERROR, std::string("Shader program linkage error:\n")+log_);
        }
    }
    glDetachShader(handle, vertexShader->handle);
    glDetachShader(handle, fragmentShader->handle);
    //LOG_OWN_ACTION(SHADER_LINKAGE, 1, "");
    ODE_CHECK_GL_ERROR();
    return ready;
}

ShaderProgram::operator bool() const {
    return ready;
}

Uniform ShaderProgram::getUniform(const char *name) const {
    Uniform output;
    output.index = glGetUniformLocation(handle, reinterpret_cast<const GLchar *>(name));
    ODE_CHECK_GL_ERROR();
    return output;
}

const char *ShaderProgram::getLog() const {
    return log.c_str();
}

void ShaderProgram::bind() const {
    glUseProgram(handle);
    ODE_CHECK_GL_ERROR();
}

}
