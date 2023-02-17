
#pragma once

#include "Vector3.h"
#include "Matrix2x2.h"
#include "Matrix3x2.h"

namespace ode {

template <typename T>
class Matrix3x3;

typedef Matrix3x3<int> Matrix3x3i;
typedef Matrix3x3<double> Matrix3x3d;
typedef Matrix3x3<float> Matrix3x3f;

/// A standard mathematical 3x3 matrix with elements of type T and overloaded arithmetic operators
template <typename T>
class Matrix3x3 {

    Vector3<T> cols[3];

public:
    constexpr Matrix3x3() { }
    constexpr explicit Matrix3x3(T diagonalValue) : cols { Vector3<T>(diagonalValue, T(), T()), Vector3<T>(T(), diagonalValue, T()), Vector3<T>(T(), T(), diagonalValue) } { }
    constexpr explicit Matrix3x3(const Vector3<T> cols[3]) : cols { cols[0], cols[1], cols[2] } { }
    constexpr Matrix3x3(const Vector3<T> &col0, const Vector3<T> &col1, const Vector3<T> &col2) : cols { col0, col1, col2 } { }
    constexpr Matrix3x3(const T values[3][3]) : cols { Vector3<T>(values[0]), Vector3<T>(values[1]), Vector3<T>(values[2]) } { }
    constexpr Matrix3x3(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22) : cols { Vector3<T>(m00, m01, m02), Vector3<T>(m10, m11, m12), Vector3<T>(m20, m21, m22) } { }
    constexpr Matrix3x3(const Matrix3x3<T> &orig) = default;
    constexpr Matrix3x3(Matrix3x3<T> &&orig) = default;
    template <typename S>
    constexpr explicit Matrix3x3(const Matrix3x3<S> &orig) : cols { Vector3<T>(orig[0]), Vector3<T>(orig[1]), Vector3<T>(orig[2]) } { }
    constexpr explicit Matrix3x3(const Matrix2x2<T> &orig) : cols { Vector3<T>(orig[0][0], orig[0][1], T()), Vector3<T>(orig[1][0], orig[1][1], T()), Vector3<T>(T(), T(), T(1)) } { }
    constexpr explicit Matrix3x3(const Matrix3x2<T> &orig) : cols { Vector3<T>(orig[0][0], orig[0][1], T()), Vector3<T>(orig[1][0], orig[1][1], T()), Vector3<T>(orig[2][0], orig[2][1], T(1)) } { }
    constexpr Matrix3x3 &operator=(const Matrix3x3<T> &orig) = default;
    constexpr Matrix3x3 &operator=(Matrix3x3<T> &&orig) = default;

    constexpr Matrix3x3<T> &operator+=(const Matrix3x3<T> &other);
    constexpr Matrix3x3<T> &operator-=(const Matrix3x3<T> &other);
    constexpr Matrix3x3<T> &operator*=(const Matrix3x3<T> &other);
    constexpr Matrix3x3<T> &operator*=(T other);
    constexpr Matrix3x3<T> &operator/=(T other);
    constexpr Matrix3x3<T> &operator%=(T other);
    constexpr const Matrix3x3<T> &operator+() const;
    constexpr Matrix3x3<T> operator-() const;
    constexpr const Vector3<T> &operator[](int col) const;
    constexpr Vector3<T> &operator[](int col);
    constexpr explicit operator Matrix3x2<T>() const;
    constexpr explicit operator bool() const;

    constexpr friend Matrix3x3<T> operator*(T a, const Matrix3x3<T> &b) {
        return Matrix3x3<T>(a*b[0][0], a*b[0][1], a*b[0][2], a*b[1][0], a*b[1][1], a*b[1][2], a*b[2][0], a*b[2][1], a*b[2][2]);
    }

    constexpr friend Matrix3x3<T> operator*(const Matrix3x3<T> &a, T b) {
        return Matrix3x3<T>(a[0][0]*b, a[0][1]*b, a[0][2]*b, a[1][0]*b, a[1][1]*b, a[1][2]*b, a[2][0]*b, a[2][1]*b, a[2][2]*b);
    }

    constexpr friend Matrix3x3<T> operator/(const Matrix3x3<T> &a, T b) {
        return Matrix3x3<T>(a[0][0]/b, a[0][1]/b, a[0][2]/b, a[1][0]/b, a[1][1]/b, a[1][2]/b, a[2][0]/b, a[2][1]/b, a[2][2]/b);
    }

    constexpr friend Matrix3x3<T> operator%(const Matrix3x3<T> &a, T b) {
        return Matrix3x3<T>(a[0][0]%b, a[0][1]%b, a[0][2]%b, a[1][0]%b, a[1][1]%b, a[1][2]%b, a[2][0]%b, a[2][1]%b, a[2][2]%b);
    }

};

template <typename T>
constexpr bool operator==(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

template <typename T>
constexpr bool operator!=(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

template <typename T>
constexpr Matrix3x3<T> operator+(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return Matrix3x3<T>(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}

template <typename T>
constexpr Matrix3x3<T> operator-(const Matrix3x3<T> &a, const Matrix3x3<T> &b) {
    return Matrix3x3<T>(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

template <typename T>
constexpr Matrix3x3<T> operator*(const Matrix3x3<T> &a, const Matrix3x3<T> &b);

template <typename T>
constexpr Vector3<T> operator*(const Matrix3x3<T> &a, const Vector3<T> &b);

template <typename T>
constexpr Vector3<T> operator*(const Vector3<T> &a, const Matrix3x3<T> &b);

template <typename T>
constexpr T determinant(const Matrix3x3<T> &matrix);

template <typename T>
constexpr Matrix3x3<T> inverse(const Matrix3x3<T> &matrix);

template <typename T>
constexpr Matrix3x3<T> transpose(const Matrix3x3<T> &matrix);

template <typename T>
constexpr Matrix3x3<T> outerProduct(const Vector3<T> &a, const Vector3<T> &b);

/// Hadamard product is simply the product of component-wise multiplication
template <typename T>
constexpr Matrix3x3<T> hadamardProduct(const Matrix3x3<T> &a, const Matrix3x3<T> &b);

}

#include "Matrix3x3.hpp"
