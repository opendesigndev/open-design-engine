
#pragma once

#include "Vector2.h"
#include "Vector3.h"

namespace ode {

template <typename T>
class Matrix3x2;

typedef Matrix3x2<int> Matrix3x2i;
typedef Matrix3x2<double> Matrix3x2d;
typedef Matrix3x2<float> Matrix3x2f;

/// A standard mathematical 3x2 matrix with elements of type T and overloaded arithmetic operators
template <typename T>
class Matrix3x2 {

    Vector2<T> cols[3];

public:
    constexpr Matrix3x2() { }
    constexpr explicit Matrix3x2(T diagonalValue) : cols { Vector2<T>(diagonalValue, T()), Vector2<T>(T(), diagonalValue), Vector2<T>(T(), T()) } { }
    constexpr explicit Matrix3x2(const Vector2<T> cols[3]) : cols { cols[0], cols[1], cols[2] } { }
    constexpr Matrix3x2(const Vector2<T> &col0, const Vector2<T> &col1, const Vector2<T> &col2) : cols { col0, col1, col2 } { }
    constexpr Matrix3x2(const T values[3][2]) : cols { Vector2<T>(values[0]), Vector2<T>(values[1]), Vector2<T>(values[2]) } { }
    constexpr explicit Matrix3x2(const T v[6]) : cols { Vector2<T>(v[0], v[1]), Vector2<T>(v[2], v[3]), Vector2<T>(v[4], v[5]) } { }
    constexpr Matrix3x2(T m00, T m01, T m10, T m11, T m20, T m21) : cols { Vector2<T>(m00, m01), Vector2<T>(m10, m11), Vector2<T>(m20, m21) } { }
    constexpr Matrix3x2(const Matrix3x2<T> &orig) = default;
    constexpr Matrix3x2(Matrix3x2<T> &&orig) = default;
    template <typename S>
    constexpr explicit Matrix3x2(const Matrix3x2<S> &orig) : cols { Vector2<T>(orig[0]), Vector2<T>(orig[1]), Vector2<T>(orig[2]) } { }
    constexpr Matrix3x2 &operator=(const Matrix3x2<T> &orig) = default;
    constexpr Matrix3x2 &operator=(Matrix3x2<T> &&orig) = default;

    constexpr Matrix3x2<T> &operator+=(const Matrix3x2<T> &other);
    constexpr Matrix3x2<T> &operator-=(const Matrix3x2<T> &other);
    constexpr Matrix3x2<T> &operator*=(T other);
    constexpr Matrix3x2<T> &operator/=(T other);
    constexpr Matrix3x2<T> &operator%=(T other);
    constexpr const Matrix3x2<T> &operator+() const;
    constexpr Matrix3x2<T> operator-() const;
    constexpr const Vector2<T> &operator[](int col) const;
    constexpr Vector2<T> &operator[](int col);
    constexpr explicit operator bool() const;

    constexpr friend Matrix3x2<T> operator*(T a, const Matrix3x2<T> &b) {
        return Matrix3x2<T>(a*b[0][0], a*b[0][1], a*b[1][0], a*b[1][1], a*b[2][0], a*b[2][1]);
    }

    constexpr friend Matrix3x2<T> operator*(const Matrix3x2<T> &a, T b) {
        return Matrix3x2<T>(a[0][0]*b, a[0][1]*b, a[1][0]*b, a[1][1]*b, a[2][0]*b, a[2][1]*b);
    }

    constexpr friend Matrix3x2<T> operator/(const Matrix3x2<T> &a, T b) {
        return Matrix3x2<T>(a[0][0]/b, a[0][1]/b, a[1][0]/b, a[1][1]/b, a[2][0]/b, a[2][1]/b);
    }

    constexpr friend Matrix3x2<T> operator%(const Matrix3x2<T> &a, T b) {
        return Matrix3x2<T>(a[0][0]%b, a[0][1]%b, a[1][0]%b, a[1][1]%b, a[2][0]%b, a[2][1]%b);
    }

};

template <typename T>
constexpr bool operator==(const Matrix3x2<T> &a, const Matrix3x2<T> &b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

template <typename T>
constexpr bool operator!=(const Matrix3x2<T> &a, const Matrix3x2<T> &b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

template <typename T>
constexpr Matrix3x2<T> operator+(const Matrix3x2<T> &a, const Matrix3x2<T> &b) {
    return Matrix3x2<T>(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}

template <typename T>
constexpr Matrix3x2<T> operator-(const Matrix3x2<T> &a, const Matrix3x2<T> &b) {
    return Matrix3x2<T>(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

template <typename T>
constexpr Vector2<T> operator*(const Matrix3x2<T> &a, const Vector3<T> &b);

template <typename T>
constexpr Vector3<T> operator*(const Vector2<T> &a, const Matrix3x2<T> &b);

template <typename T>
constexpr Matrix3x2<T> outerProduct(const Vector2<T> &a, const Vector3<T> &b);

/// Hadamard product is simply the product of component-wise multiplication
template <typename T>
constexpr Matrix3x2<T> hadamardProduct(const Matrix3x2<T> &a, const Matrix3x2<T> &b);

}

#include "Matrix3x2.hpp"
