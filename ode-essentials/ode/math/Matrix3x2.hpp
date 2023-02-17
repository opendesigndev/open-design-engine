
#include "Matrix3x2.h"

#include "../utils.h"

namespace ode {

template <typename T>
constexpr Matrix3x2<T> &Matrix3x2<T>::operator+=(const Matrix3x2<T> &other) {
    cols[0] += other.cols[0];
    cols[1] += other.cols[1];
    cols[2] += other.cols[2];
    return *this;
}

template <typename T>
constexpr Matrix3x2<T> &Matrix3x2<T>::operator-=(const Matrix3x2<T> &other) {
    cols[0] -= other.cols[0];
    cols[1] -= other.cols[1];
    cols[2] -= other.cols[2];
    return *this;
}

template <typename T>
constexpr Matrix3x2<T> &Matrix3x2<T>::operator*=(T other) {
    cols[0] *= other;
    cols[1] *= other;
    cols[2] *= other;
    return *this;
}

template <typename T>
constexpr Matrix3x2<T> &Matrix3x2<T>::operator/=(T other) {
    cols[0] /= other;
    cols[1] /= other;
    cols[2] /= other;
    return *this;
}

template <typename T>
constexpr Matrix3x2<T> &Matrix3x2<T>::operator%=(T other) {
    cols[0] %= other;
    cols[1] %= other;
    cols[2] %= other;
    return *this;
}

template <typename T>
constexpr const Matrix3x2<T> &Matrix3x2<T>::operator+() const {
    return *this;
}

template <typename T>
constexpr Matrix3x2<T> Matrix3x2<T>::operator-() const {
    return Matrix3x2<T>(-cols[0], -cols[1], -cols[2]);
}

template <typename T>
constexpr const Vector2<T> &Matrix3x2<T>::operator[](int col) const {
    ODE_ASSERT(col >= 0 && col < 3);
    return cols[col];
}

template <typename T>
constexpr Vector2<T> &Matrix3x2<T>::operator[](int col) {
    ODE_ASSERT(col >= 0 && col < 3);
    return cols[col];
}

template <typename T>
constexpr Matrix3x2<T>::operator bool() const {
    return cols[0] || cols[1] || cols[2];
}

template <typename T>
constexpr Vector2<T> operator*(const Matrix3x2<T> &a, const Vector3<T> &b) {
    return Vector2<T>(
        a[0][0]*b.x + a[1][0]*b.y + a[2][0]*b.z,
        a[0][1]*b.x + a[1][1]*b.y + a[2][1]*b.z
    );
}

template <typename T>
constexpr Vector3<T> operator*(const Vector2<T> &a, const Matrix3x2<T> &b) {
    return Vector3<T>(
        a.x*b[0][0] + a.y*b[0][1],
        a.x*b[1][0] + a.y*b[1][1],
        a.x*b[2][0] + a.y*b[2][1]
    );
}

template <typename T>
constexpr Matrix3x2<T> outerProduct(const Vector2<T> &a, const Vector3<T> &b) {
    return Matrix3x2<T>(
        a.x*b.x, a.y*b.x,
        a.x*b.y, a.y*b.y,
        a.x*b.z, a.y*b.z
    );
}

template <typename T>
constexpr Matrix3x2<T> hadamardProduct(const Matrix3x2<T> &a, const Matrix3x2<T> &b) {
    return Matrix3x2<T>(a[0]*b[0], a[1]*b[1], a[2]*b[2]);
}

}
