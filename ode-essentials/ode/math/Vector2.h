
#pragma once

namespace ode {

template <typename T>
struct Vector2;

typedef Vector2<int> Vector2i;
typedef Vector2<double> Vector2d;
typedef Vector2<float> Vector2f;

/// A standard mathematical 2-dimensional vector with elements of type T and overloaded arithmetic operators
template <typename T>
struct Vector2 {

    T x, y;

    constexpr Vector2() : x(), y() { }
    constexpr explicit Vector2(T fillValue) : x(fillValue), y(fillValue) { }
    constexpr explicit Vector2(const T values[2]) : x(values[0]), y(values[1]) { }
    constexpr Vector2(T x, T y) : x(x), y(y) { }
    constexpr Vector2(const Vector2<T> &orig) = default;
    constexpr Vector2(Vector2<T> &&orig) = default;
    template <typename S>
    constexpr explicit Vector2(const Vector2<S> &orig) : x(T(orig.x)), y(T(orig.y)) { }
    constexpr Vector2 &operator=(const Vector2<T> &orig) = default;
    constexpr Vector2 &operator=(Vector2<T> &&orig) = default;

    constexpr Vector2<T> &operator+=(const Vector2<T> &other);
    constexpr Vector2<T> &operator-=(const Vector2<T> &other);
    constexpr Vector2<T> &operator*=(const Vector2<T> &other);
    constexpr Vector2<T> &operator/=(const Vector2<T> &other);
    constexpr Vector2<T> &operator%=(const Vector2<T> &other);
    constexpr Vector2<T> &operator*=(T other);
    constexpr Vector2<T> &operator/=(T other);
    constexpr Vector2<T> &operator%=(T other);
    constexpr const Vector2<T> &operator+() const;
    constexpr Vector2<T> operator-() const;
    constexpr T operator[](int i) const;
    constexpr T &operator[](int i);
    constexpr explicit operator bool() const;

    constexpr T squaredLength() const;
    constexpr T length() const;
    /// Returns unit vector with the same direction or zeroResult for null vector
    constexpr Vector2<T> normalized(const Vector2<T> &zeroResult = Vector2<T>(T(1), T(0))) const;
    /// Returns vector with the same length that is orthogonal to this, polarity (1 or -1) dictates direction
    constexpr Vector2<T> orthogonal(int polarity = 1) const;
    /// Returns unit vector that is orthogonal to this or zeroResult for null vector, polarity (1 or -1) dictates direction
    constexpr Vector2<T> orthonormal(int polarity = 1, const Vector2<T> &zeroResult = Vector2<T>(T(1), T(0))) const;

    constexpr friend Vector2<T> operator*(T a, const Vector2<T> &b) {
        return Vector2<T>(a*b.x, a*b.y);
    }

    constexpr friend Vector2<T> operator*(const Vector2<T> &a, T b) {
        return Vector2<T>(a.x*b, a.y*b);
    }

    constexpr friend Vector2<T> operator/(const Vector2<T> &a, T b) {
        return Vector2<T>(a.x/b, a.y/b);
    }

    constexpr friend Vector2<T> operator%(const Vector2<T> &a, T b) {
        return Vector2<T>(a.x%b, a.y%b);
    }

};

template <typename T>
constexpr bool operator==(const Vector2<T> &a, const Vector2<T> &b) {
    return a.x == b.x && a.y == b.y;
}

template <typename T>
constexpr bool operator!=(const Vector2<T> &a, const Vector2<T> &b) {
    return a.x != b.x || a.y != b.y;
}

template <typename T>
constexpr Vector2<T> operator+(const Vector2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(a.x+b.x, a.y+b.y);
}

template <typename T>
constexpr Vector2<T> operator-(const Vector2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(a.x-b.x, a.y-b.y);
}

/// Component-wise multiplication, for the dot product use dotProduct(a, b)
template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(a.x*b.x, a.y*b.y);
}

template <typename T>
constexpr Vector2<T> operator/(const Vector2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(a.x/b.x, a.y/b.y);
}

template <typename T>
constexpr Vector2<T> operator%(const Vector2<T> &a, const Vector2<T> &b) {
    return Vector2<T>(a.x%b.x, a.y%b.y);
}

template <typename T>
constexpr T dotProduct(const Vector2<T> &a, const Vector2<T> &b) {
    return a.x*b.x+a.y*b.y;
}

/// Non-standard 2D cross product is a.x*b.y - a.y*b.x
template <typename T>
constexpr T crossProduct(const Vector2<T> &a, const Vector2<T> &b) {
    return a.x*b.y-a.y*b.x;
}

}

#include "Vector2.hpp"
