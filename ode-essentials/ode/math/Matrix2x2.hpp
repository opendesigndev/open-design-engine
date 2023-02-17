
#include "Matrix2x2.h"

#include "../utils.h"

namespace ode {

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator+=(const Matrix2x2<T> &other) {
    cols[0] += other.cols[0];
    cols[1] += other.cols[1];
    return *this;
}

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator-=(const Matrix2x2<T> &other) {
    cols[0] -= other.cols[0];
    cols[1] -= other.cols[1];
    return *this;
}

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator*=(const Matrix2x2<T> &other) {
    return *this = *this*other;
}

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator*=(T other) {
    cols[0] *= other;
    cols[1] *= other;
    return *this;
}

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator/=(T other) {
    cols[0] /= other;
    cols[1] /= other;
    return *this;
}

template <typename T>
constexpr Matrix2x2<T> &Matrix2x2<T>::operator%=(T other) {
    cols[0] %= other;
    cols[1] %= other;
    return *this;
}

template <typename T>
constexpr const Matrix2x2<T> &Matrix2x2<T>::operator+() const {
    return *this;
}

template <typename T>
constexpr Matrix2x2<T> Matrix2x2<T>::operator-() const {
    return Matrix2x2<T>(-cols[0], -cols[1]);
}

template <typename T>
constexpr const Vector2<T> &Matrix2x2<T>::operator[](int col) const {
    ODE_ASSERT(col >= 0 && col < 2);
    return cols[col];
}

template <typename T>
constexpr Vector2<T> &Matrix2x2<T>::operator[](int col) {
    ODE_ASSERT(col >= 0 && col < 2);
    return cols[col];
}

template <typename T>
constexpr Matrix2x2<T>::operator bool() const {
    return cols[0] || cols[1];
}

template <typename T>
constexpr Matrix2x2<T> operator*(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return Matrix2x2<T>(
        a[0][0]*b[0][0] + a[1][0]*b[0][1],
        a[0][1]*b[0][0] + a[1][1]*b[0][1],
        a[0][0]*b[1][0] + a[1][0]*b[1][1],
        a[0][1]*b[1][0] + a[1][1]*b[1][1]
    );
}

template <typename T>
constexpr Vector2<T> operator*(const Matrix2x2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(
        a[0][0]*b.x + a[1][0]*b.y,
        a[0][1]*b.x + a[1][1]*b.y
    );
}

template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &a, const Matrix2x2<T> &b) {
    return Vector2<T>(
        a.x*b[0][0] + a.y*b[0][1],
        a.x*b[1][0] + a.y*b[1][1]
    );
}

template <typename T>
constexpr T determinant(const Matrix2x2<T> &matrix) {
    return matrix[0][0]*matrix[1][1]-matrix[0][1]*matrix[1][0];
}

template <typename T>
constexpr Matrix2x2<T> inverse(const Matrix2x2<T> &matrix) {
    T det = determinant(matrix);
    return Matrix2x2<T>(
        matrix[1][1]/det, -matrix[0][1]/det,
        -matrix[1][0]/det, matrix[0][0]/det
    );
}

template <typename T>
constexpr Matrix2x2<T> transpose(const Matrix2x2<T> &matrix) {
    return Matrix2x2<T>(
        matrix[0][0], matrix[1][0],
        matrix[0][1], matrix[1][1]
    );
}

template <typename T>
constexpr Matrix2x2<T> outerProduct(const Vector2<T> &a, const Vector2<T> &b) {
    return Matrix2x2<T>(
        a.x*b.x, a.y*b.x,
        a.x*b.y, a.y*b.y
    );
}

template <typename T>
constexpr Matrix2x2<T> hadamardProduct(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return Matrix2x2<T>(a[0]*b[0], a[1]*b[1]);
}

}
