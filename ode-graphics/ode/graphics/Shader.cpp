
#include "Shader.h"

namespace ode {

Shader::Shader(const char *label) : handle(0), ready(false), label(label) { }

Shader::~Shader() {
    if (handle) {
        glDeleteShader(handle);
        ODE_CHECK_GL_ERROR();
    }
}

bool Shader::initialize(const GLchar *const *sources, const GLint *lengths, size_t count) {
    if (ready)
        return false;
    if (!handle)
        handle = glCreateShader(shaderType());
    if (!handle)
        return false;
#if defined(__EMSCRIPTEN__) && defined(ODE_DEBUG)
    // This seems to happen to some Chrome users for unknown and strange reasons, TODO gather more info such as which
    if (!glIsShader(handle)) {
        const char *shaderTypeName = shaderType() == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex";
        //Log::instance.logf(Log::CORE_OPENGL, Log::ERROR, "%s shader handle breakdown before compile: %s", shaderTypeName, label);
    }
#endif

    glShaderSource(handle, (GLsizei) count, const_cast<const GLchar **>(sources), lengths);
    glCompileShader(handle);
    ODE_CHECK_GL_ERROR();

    GLint status = GL_FALSE;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
#ifndef ODE_DEBUG
    if (status != GL_TRUE)
#endif
    {
        GLint infoLogLength = -1;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0) {
            log.resize(infoLogLength);
            glGetShaderInfoLog(handle, infoLogLength, nullptr, &log[0]);
            fprintf(stderr, "Shader compilation error (%s):\n%s", label, log.c_str());
            //Log::instance.logf(Log::CORE_OPENGL, Log::ERROR, "Shader compilation error (%s):\n%s", label_, log_.c_str());
        }
        ODE_CHECK_GL_ERROR();
        #ifdef ODE_DEBUG
        if (status != GL_TRUE)
        #endif
        return false;
    }
#ifdef ODE_DEBUG
    int totalSrcLen = 0;
    for (size_t i = 0; i < count; ++i)
        totalSrcLen += lengths[i];
    //LOG_OWN_ACTION(SHADER_COMPILATION, totalSrcLen, "");
#endif
    ODE_CHECK_GL_ERROR();
    return ready = true;
}

Shader::operator bool() const {
    return ready;
}

const char *Shader::getLog() const {
    return log.c_str();
}

VertexShader::VertexShader(const char *label) : Shader(label) { }

FragmentShader::FragmentShader(const char *label) : Shader(label) { }

GLenum VertexShader::shaderType() const {
    return GL_VERTEX_SHADER;
}

GLenum FragmentShader::shaderType() const {
    return GL_FRAGMENT_SHADER;
}

}
