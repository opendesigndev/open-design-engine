
#pragma once

#include <ode/graphics/Color.h>
#include "gl.h"

namespace ode {

class ShaderProgram;

/// Provides access to a shader program's uniform variable
class Uniform {
    friend class ShaderProgram;

public:
    Uniform();
    bool setFloat(float value);
    bool setFloat(const float *value, int n);
    bool setVec2(const float *value, int n = 1);
    bool setVec3(const float *value, int n = 1);
    bool setVec4(const float *value, int n = 1);
    bool setMat2(const float *value, int n = 1);
    bool setMat3(const float *value, int n = 1);
    bool setMat4(const float *value, int n = 1);
    bool setInt(int value);
    bool setIvec2(const int *value, int n = 1);
    bool setIvec3(const int *value, int n = 1);
    bool setIvec4(const int *value, int n = 1);
    bool setBool(bool value);
    bool setColor(const Color &color);

private:
    GLint index;

};

}
