
#include "Uniform.h"

namespace ode {

Uniform::Uniform() {
    index = -1;
}

#define UNIF_CHK(call) if (index < 0) return false; call; ODE_CHECK_GL_ERROR(); return true;

bool Uniform::setFloat(float value) {
    UNIF_CHK(glUniform1f(index, value));
}

bool Uniform::setFloat(const float *value, int n) {
    UNIF_CHK(glUniform1fv(index, n, value));
}

bool Uniform::setVec2(const float *value, int n) {
    UNIF_CHK(glUniform2fv(index, n, value));
}

bool Uniform::setVec3(const float *value, int n) {
    UNIF_CHK(glUniform3fv(index, n, value));
}

bool Uniform::setVec4(const float *value, int n) {
    UNIF_CHK(glUniform4fv(index, n, value));
}

bool Uniform::setMat2(const float *value, int n) {
    UNIF_CHK(glUniformMatrix2fv(index, n, GL_FALSE, value));
}

bool Uniform::setMat3(const float *value, int n) {
    UNIF_CHK(glUniformMatrix3fv(index, n, GL_FALSE, value));
}

bool Uniform::setMat4(const float *value, int n) {
    UNIF_CHK(glUniformMatrix4fv(index, n, GL_FALSE, value));
}

bool Uniform::setInt(int value) {
    UNIF_CHK(glUniform1i(index, value));
}

bool Uniform::setIvec2(const int *value, int n) {
    UNIF_CHK(glUniform2iv(index, n, value));
}

bool Uniform::setIvec3(const int *value, int n) {
    UNIF_CHK(glUniform3iv(index, n, value));
}

bool Uniform::setIvec4(const int *value, int n) {
    UNIF_CHK(glUniform4iv(index, n, value));
}

bool Uniform::setBool(bool value) {
    UNIF_CHK(glUniform1i(index, value ? 1 : 0));
}

bool Uniform::setColor(const Color &color) {
    UNIF_CHK(glUniform4f(index, float(color.r), float(color.g), float(color.b), float(color.a)));
}

}
