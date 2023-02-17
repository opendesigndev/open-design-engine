
#pragma once

#include <string>
#include "gl.h"
#include "Shader.h"
#include "Uniform.h"

namespace ode {

/// Represents an OpenGL shader program
class ShaderProgram {

public:
    ShaderProgram();
    ShaderProgram(const ShaderProgram &) = delete;
    ~ShaderProgram();
    ShaderProgram &operator=(const ShaderProgram &) = delete;
    /// Initializes using a vertex and a fragment shader component
    bool initialize(const VertexShader *vertexShader, const FragmentShader *fragmentShader);
    /// Returns true if successfully initialized
    explicit operator bool() const;
    /// Returns an accessor object of a uniform variable with the specified name
    Uniform getUniform(const char *name) const;
    /// Returns link error log string
    const char *getLog() const;
    /// Binds the program as the active shader program used for subsequent draw calls
    void bind() const;

private:
    GLuint handle;
    bool ready;
    std::string log;

};

}
