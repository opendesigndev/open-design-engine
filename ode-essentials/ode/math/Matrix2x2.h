
#pragma once

#include "Vector2.h"

namespace ode {

template <typename T>
class Matrix2x2;

typedef Matrix2x2<int> Matrix2x2i;
typedef Matrix2x2<double> Matrix2x2d;
typedef Matrix2x2<float> Matrix2x2f;

/// A standard mathematical 2x2 matrix with elements of type T and overloaded arithmetic operators
template <typename T>
class Matrix2x2 {

    Vector2<T> cols[2];

public:
    constexpr Matrix2x2() { }
    constexpr explicit Matrix2x2(T diagonalValue) : cols { Vector2<T>(diagonalValue, T()), Vector2<T>(T(), diagonalValue) } { }
    constexpr explicit Matrix2x2(const Vector2<T> cols[2]) : cols { cols[0], cols[1] } { }
    constexpr Matrix2x2(const Vector2<T> &col0, const Vector2<T> &col1) : cols { col0, col1 } { }
    constexpr Matrix2x2(const T values[2][2]) : cols { Vector2<T>(values[0]), Vector2<T>(values[1]) } { }
    constexpr Matrix2x2(T m00, T m01, T m10, T m11) : cols { Vector2<T>(m00, m01), Vector2<T>(m10, m11) } { }
    constexpr Matrix2x2(const Matrix2x2<T> &orig) = default;
    constexpr Matrix2x2(Matrix2x2<T> &&orig) = default;
    template <typename S>
    constexpr explicit Matrix2x2(const Matrix2x2<S> &orig) : cols { Vector2<T>(orig[0]), Vector2<T>(orig[1]) } { }
    constexpr Matrix2x2 &operator=(const Matrix2x2<T> &orig) = default;
    constexpr Matrix2x2 &operator=(Matrix2x2<T> &&orig) = default;

    constexpr Matrix2x2<T> &operator+=(const Matrix2x2<T> &other);
    constexpr Matrix2x2<T> &operator-=(const Matrix2x2<T> &other);
    constexpr Matrix2x2<T> &operator*=(const Matrix2x2<T> &other);
    constexpr Matrix2x2<T> &operator*=(T other);
    constexpr Matrix2x2<T> &operator/=(T other);
    constexpr Matrix2x2<T> &operator%=(T other);
    constexpr const Matrix2x2<T> &operator+() const;
    constexpr Matrix2x2<T> operator-() const;
    constexpr const Vector2<T> &operator[](int col) const;
    constexpr Vector2<T> &operator[](int col);
    constexpr explicit operator bool() const;

    constexpr friend Matrix2x2<T> operator*(T a, const Matrix2x2<T> &b) {
        return Matrix2x2<T>(a*b[0][0], a*b[0][1], a*b[1][0], a*b[1][1]);
    }

    constexpr friend Matrix2x2<T> operator*(const Matrix2x2<T> &a, T b) {
        return Matrix2x2<T>(a[0][0]*b, a[0][1]*b, a[1][0]*b, a[1][1]*b);
    }

    constexpr friend Matrix2x2<T> operator/(const Matrix2x2<T> &a, T b) {
        return Matrix2x2<T>(a[0][0]/b, a[0][1]/b, a[1][0]/b, a[1][1]/b);
    }

    constexpr friend Matrix2x2<T> operator%(const Matrix2x2<T> &a, T b) {
        return Matrix2x2<T>(a[0][0]%b, a[0][1]%b, a[1][0]%b, a[1][1]%b);
    }

};

template <typename T>
constexpr bool operator==(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return a[0] == b[0] && a[1] == b[1];
}

template <typename T>
constexpr bool operator!=(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return a[0] != b[0] || a[1] != b[1];
}

template <typename T>
constexpr Matrix2x2<T> operator+(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return Matrix2x2<T>(a[0]+b[0], a[1]+b[1]);
}

template <typename T>
constexpr Matrix2x2<T> operator-(const Matrix2x2<T> &a, const Matrix2x2<T> &b) {
    return Matrix2x2<T>(a[0]-b[0], a[1]-b[1]);
}

template <typename T>
constexpr Matrix2x2<T> operator*(const Matrix2x2<T> &a, const Matrix2x2<T> &b);

template <typename T>
constexpr Vector2<T> operator*(const Matrix2x2<T> &a, const Vector2<T> &b);

template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &a, const Matrix2x2<T> &b);

template <typename T>
constexpr T determinant(const Matrix2x2<T> &matrix);

template <typename T>
constexpr Matrix2x2<T> inverse(const Matrix2x2<T> &matrix);

template <typename T>
constexpr Matrix2x2<T> transpose(const Matrix2x2<T> &matrix);

template <typename T>
constexpr Matrix2x2<T> outerProduct(const Vector2<T> &a, const Vector2<T> &b);

/// Hadamard product is simply the product of component-wise multiplication
template <typename T>
constexpr Matrix2x2<T> hadamardProduct(const Matrix2x2<T> &a, const Matrix2x2<T> &b);

}

#include "Matrix2x2.hpp"
