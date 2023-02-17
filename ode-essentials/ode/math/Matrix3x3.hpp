
#include "Matrix3x3.h"

#include "../utils.h"

namespace ode {

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator+=(const Matrix3x3<T> &other) {
    cols[0] += other.cols[0];
    cols[1] += other.cols[1];
    cols[2] += other.cols[2];
    return *this;
}

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator-=(const Matrix3x3<T> &other) {
    cols[0] -= other.cols[0];
    cols[1] -= other.cols[1];
    cols[2] -= other.cols[2];
    return *this;
}

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator*=(const Matrix3x3<T> &other) {
    return *this = *this*other;
}

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator*=(T other) {
    cols[0] *= other;
    cols[1] *= other;
    cols[2] *= other;
    return *this;
}

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator/=(T other) {
    cols[0] /= other;
    cols[1] /= other;
    cols[2] /= other;
    return *this;
}

template <typename T>
constexpr Matrix3x3<T> &Matrix3x3<T>::operator%=(T other) {
    cols[0] %= other;
    cols[1] %= other;
    cols[2] %= other;
    return *this;
}

template <typename T>
constexpr const Matrix3x3<T> &Matrix3x3<T>::operator+() const {
    return *this;
}

template <typename T>
constexpr Matrix3x3<T> Matrix3x3<T>::operator-() const {
    return Matrix3x3<T>(-cols[0], -cols[1], -cols[2]);
}

template <typename T>
constexpr const Vector3<T> &Matrix3x3<T>::operator[](int col) const {
    ODE_ASSERT(col >= 0 && col < 3);
    return cols[col];
}

template <typename T>
constexpr Vector3<T> &Matrix3x3<T>::operator[](int col) {
    ODE_ASSERT(col >= 0 && col < 3);
    return cols[col];
}

template <typename T>
constexpr Matrix3x3<T>::operator Matrix3x2<T>() const {
    return Matrix3x2<T>(cols[0][0], cols[0][1], cols[1][0], cols[1][1], cols[2][0], cols[2][1]);
}

template <typename T>
constexpr Matrix3x3<T>::operator bool() const {
    return cols[0] || cols[1] || cols[2];
}

template <typename T>
constexpr Matrix3x3<T> operator*(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return Matrix3x3<T>(
        a[0][0]*b[0][0] + a[1][0]*b[0][1] + a[2][0]*b[0][2],
        a[0][1]*b[0][0] + a[1][1]*b[0][1] + a[2][1]*b[0][2],
        a[0][2]*b[0][0] + a[1][2]*b[0][1] + a[2][2]*b[0][2],
        a[0][0]*b[1][0] + a[1][0]*b[1][1] + a[2][0]*b[1][2],
        a[0][1]*b[1][0] + a[1][1]*b[1][1] + a[2][1]*b[1][2],
        a[0][2]*b[1][0] + a[1][2]*b[1][1] + a[2][2]*b[1][2],
        a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0]*b[2][2],
        a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]*b[2][2],
        a[0][2]*b[2][0] + a[1][2]*b[2][1] + a[2][2]*b[2][2]
    );
}

template <typename T>
constexpr Vector3<T> operator*(const Matrix3x3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(
        a[0][0]*b.x + a[1][0]*b.y + a[2][0]*b.z,
        a[0][1]*b.x + a[1][1]*b.y + a[2][1]*b.z,
        a[0][2]*b.x + a[1][2]*b.y + a[2][2]*b.z
    );
}

template <typename T>
constexpr Vector3<T> operator*(const Vector3<T> &a, const Matrix3x3<T> &b) {
    return Vector3<T>(
        a.x*b[0][0] + a.y*b[0][1] + a.z*b[0][2],
        a.x*b[1][0] + a.y*b[1][1] + a.z*b[1][2],
        a.x*b[2][0] + a.y*b[2][1] + a.z*b[2][2]
    );
}

template <typename T>
constexpr T determinant(const Matrix3x3<T> &matrix) {
    return (
        matrix[0][0]*(matrix[1][1]*matrix[2][2]-matrix[2][1]*matrix[1][2])+
        matrix[1][0]*(matrix[2][1]*matrix[0][2]-matrix[0][1]*matrix[2][2])+
        matrix[2][0]*(matrix[0][1]*matrix[1][2]-matrix[1][1]*matrix[0][2])
    );
}

template <typename T>
constexpr Matrix3x3<T> inverse(const Matrix3x3<T> &m) {
    T det = determinant(m);
    return Matrix3x3<T>(
        (m[1][1]*m[2][2]-m[1][2]*m[2][1])/det, (m[0][2]*m[2][1]-m[0][1]*m[2][2])/det, (m[0][1]*m[1][2]-m[0][2]*m[1][1])/det,
        (m[1][2]*m[2][0]-m[1][0]*m[2][2])/det, (m[0][0]*m[2][2]-m[0][2]*m[2][0])/det, (m[0][2]*m[1][0]-m[0][0]*m[1][2])/det,
        (m[1][0]*m[2][1]-m[1][1]*m[2][0])/det, (m[0][1]*m[2][0]-m[0][0]*m[2][1])/det, (m[0][0]*m[1][1]-m[0][1]*m[1][0])/det
    );
}

template <typename T>
constexpr Matrix3x3<T> transpose(const Matrix3x3<T> &matrix) {
    return Matrix3x3<T>(
        matrix[0][0], matrix[1][0], matrix[2][0],
        matrix[0][1], matrix[1][1], matrix[2][1],
        matrix[0][2], matrix[1][2], matrix[2][2]
    );
}

template <typename T>
constexpr Matrix3x3<T> outerProduct(const Vector3<T> &a, const Vector3<T> &b) {
    return Matrix3x3<T>(
        a.x*b.x, a.y*b.x, a.z*b.x,
        a.x*b.y, a.y*b.y, a.z*b.y,
        a.x*b.z, a.y*b.z, a.z*b.z
    );
}

template <typename T>
constexpr Matrix3x3<T> hadamardProduct(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return Matrix3x3<T>(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
}

}
