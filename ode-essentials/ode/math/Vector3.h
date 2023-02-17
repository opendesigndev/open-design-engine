
#pragma once

namespace ode {

template <typename T>
struct Vector3;

typedef Vector3<int> Vector3i;
typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3f;

/// A standard mathematical 3-dimensional vector with elements of type T and overloaded arithmetic operators
template <typename T>
struct Vector3 {

    T x, y, z;

    constexpr Vector3() : x(), y(), z() { }
    constexpr explicit Vector3(T fillValue) : x(fillValue), y(fillValue), z(fillValue) { }
    constexpr explicit Vector3(const T values[3]) : x(values[0]), y(values[1]), z(values[2]) { }
    constexpr Vector3(T x, T y, T z) : x(x), y(y), z(z) { }
    constexpr Vector3(const Vector3<T> &orig) = default;
    constexpr Vector3(Vector3<T> &&orig) = default;
    template <typename S>
    constexpr explicit Vector3(const Vector3<S> &orig) : x(T(orig.x)), y(T(orig.y)), z(T(orig.z)) { }
    constexpr Vector3 &operator=(const Vector3<T> &orig) = default;
    constexpr Vector3 &operator=(Vector3<T> &&orig) = default;

    constexpr Vector3<T> &operator+=(const Vector3<T> &other);
    constexpr Vector3<T> &operator-=(const Vector3<T> &other);
    constexpr Vector3<T> &operator*=(const Vector3<T> &other);
    constexpr Vector3<T> &operator/=(const Vector3<T> &other);
    constexpr Vector3<T> &operator%=(const Vector3<T> &other);
    constexpr Vector3<T> &operator*=(T other);
    constexpr Vector3<T> &operator/=(T other);
    constexpr Vector3<T> &operator%=(T other);
    constexpr const Vector3<T> &operator+() const;
    constexpr Vector3<T> operator-() const;
    constexpr T operator[](int i) const;
    constexpr T &operator[](int i);
    constexpr explicit operator bool() const;

    constexpr T squaredLength() const;
    constexpr T length() const;
    /// Returns unit vector with the same direction or zeroResult for null vector
    constexpr Vector3<T> normalized(const Vector3<T> &zeroResult = Vector3<T>(T(1), T(0))) const;

    constexpr friend Vector3<T> operator*(T a, const Vector3<T> &b) {
        return Vector3<T>(a*b.x, a*b.y, a*b.z);
    }

    constexpr friend Vector3<T> operator*(const Vector3<T> &a, T b) {
        return Vector3<T>(a.x*b, a.y*b, a.z*b);
    }

    constexpr friend Vector3<T> operator/(const Vector3<T> &a, T b) {
        return Vector3<T>(a.x/b, a.y/b, a.z/b);
    }

    constexpr friend Vector3<T> operator%(const Vector3<T> &a, T b) {
        return Vector3<T>(a.x%b, a.y%b, a.z%b);
    }

};

template <typename T>
constexpr bool operator==(const Vector3<T> &a, const Vector3<T> &b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
constexpr bool operator!=(const Vector3<T> &a, const Vector3<T> &b) {
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

template <typename T>
constexpr Vector3<T> operator+(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.x+b.x, a.y+b.y, a.z+b.z);
}

template <typename T>
constexpr Vector3<T> operator-(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.x-b.x, a.y-b.y, a.z-b.z);
}

/// Component-wise multiplication, for the dot product use dotProduct(a, b)
template <typename T>
constexpr Vector3<T> operator*(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.x*b.x, a.y*b.y, a.z*b.z);
}

template <typename T>
constexpr Vector3<T> operator/(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.x/b.x, a.y/b.y, a.z/b.z);
}

template <typename T>
constexpr Vector3<T> operator%(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.x%b.x, a.y%b.y, a.z%b.z);
}

template <typename T>
constexpr T dotProduct(const Vector3<T> &a, const Vector3<T> &b) {
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

template <typename T>
constexpr Vector3<T> crossProduct(const Vector3<T> &a, const Vector3<T> &b) {
    return Vector3<T>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

}

#include "Vector3.hpp"
