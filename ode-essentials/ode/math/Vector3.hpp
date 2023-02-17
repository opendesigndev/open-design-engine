
#include "Vector3.h"

#include <cmath>
#include "../utils.h"

namespace ode {

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator+=(const Vector3<T> &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator-=(const Vector3<T> &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator*=(const Vector3<T> &other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator/=(const Vector3<T> &other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator%=(const Vector3<T> &other) {
    x %= other.x;
    y %= other.y;
    z %= other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator*=(T other) {
    x *= other;
    y *= other;
    z *= other;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator/=(T other) {
    x /= other;
    y /= other;
    z /= other;
    return *this;
}

template <typename T>
constexpr Vector3<T> &Vector3<T>::operator%=(T other) {
    x %= other;
    y %= other;
    z %= other;
    return *this;
}

template <typename T>
constexpr const Vector3<T> &Vector3<T>::operator+() const {
    return *this;
}

template <typename T>
constexpr Vector3<T> Vector3<T>::operator-() const {
    return Vector3<T>(-x, -y, -z);
}

template <typename T>
constexpr T Vector3<T>::operator[](int i) const {
    switch (i) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
    }
    ODE_ASSERT(i >= 0 && i < 3);
    return T();
}

template <typename T>
T &_vector3_dummy_value() {
    static T x = T();
    return x;
}

template <typename T>
constexpr T &Vector3<T>::operator[](int i) {
    switch (i) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
    }
    ODE_ASSERT(i >= 0 && i < 3);
    return _vector3_dummy_value<T>();
}

template <typename T>
constexpr Vector3<T>::operator bool() const {
    return x || y || z;
}

template <typename T>
constexpr T Vector3<T>::squaredLength() const {
    return x*x+y*y+z*z;
}

template <typename T>
constexpr T Vector3<T>::length() const {
    return T(std::sqrt(x*x+y*y+z*z));
}

template <typename T>
constexpr Vector3<T> Vector3<T>::normalized(const Vector3<T> &zeroResult) const {
    if (T len = length())
        return *this/len;
    return zeroResult;
}

}
