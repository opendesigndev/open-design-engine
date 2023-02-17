
#pragma once

#include <string>
#include "gl.h"

#ifdef __EMSCRIPTEN__
    #define ODE_GLOBAL_SHADER_PREAMBLE \
        "#version 100\n" \
        "precision mediump float;\n"
#else
    #define ODE_GLOBAL_SHADER_PREAMBLE \
        "#version 120\n"
#endif

// For #version 100 - 120
#define ODE_GLSL_VATTRIB "attribute "
#define ODE_GLSL_VVARYING "varying "
#define ODE_GLSL_FVARYING "varying "
#define ODE_GLSL_FRAGCOLOR "gl_FragColor"
#define ODE_GLSL_TEXTURE2D "texture2D"

namespace ode {

class ShaderProgram;

/// Represents an OpenGL shader component
class Shader {
    friend class ShaderProgram;

public:
    explicit Shader(const char *label);
    Shader(const Shader &) = delete;
    virtual ~Shader();
    Shader &operator=(const Shader &) = delete;
    bool initialize(const GLchar *const *sources, const GLint *lengths, size_t count);
    explicit operator bool() const;
    const char *getLog() const;

protected:
    virtual GLenum shaderType() const = 0;

private:
    GLuint handle;
    bool ready;
    const char *label;
    std::string log;

};

/// Represents an OpenGL vertex shader component
class VertexShader : public Shader {

public:
    explicit VertexShader(const char *label);
    VertexShader(const VertexShader &) = delete;
    VertexShader &operator=(const VertexShader &) = delete;

protected:
    virtual GLenum shaderType() const override;

};

/// Represents an OpenGL fragment shader component
class FragmentShader : public Shader {
public:
    explicit FragmentShader(const char *label);
    FragmentShader(const FragmentShader &) = delete;
    FragmentShader &operator=(const FragmentShader &) = delete;

protected:
    virtual GLenum shaderType() const override;

};

}
