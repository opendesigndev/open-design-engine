
#include "Vector2.h"

#include <cmath>
#include "../utils.h"

namespace ode {

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator+=(const Vector2<T> &other) {
    x += other.x;
    y += other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator-=(const Vector2<T> &other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator*=(const Vector2<T> &other) {
    x *= other.x;
    y *= other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator/=(const Vector2<T> &other) {
    x /= other.x;
    y /= other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator%=(const Vector2<T> &other) {
    x %= other.x;
    y %= other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator*=(T other) {
    x *= other;
    y *= other;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator/=(T other) {
    x /= other;
    y /= other;
    return *this;
}

template <typename T>
constexpr Vector2<T> &Vector2<T>::operator%=(T other) {
    x %= other;
    y %= other;
    return *this;
}

template <typename T>
constexpr const Vector2<T> &Vector2<T>::operator+() const {
    return *this;
}

template <typename T>
constexpr Vector2<T> Vector2<T>::operator-() const {
    return Vector2<T>(-x, -y);
}

template <typename T>
constexpr T Vector2<T>::operator[](int i) const {
    switch (i) {
        case 0: return x;
        case 1: return y;
    }
    ODE_ASSERT(i >= 0 && i < 2);
    return T();
}

template <typename T>
static T &_vector2_dummy_value() {
    static T x = T();
    return x;
}

template <typename T>
constexpr T &Vector2<T>::operator[](int i) {
    switch (i) {
        case 0: return x;
        case 1: return y;
    }
    ODE_ASSERT(i >= 0 && i < 2);
    return _vector2_dummy_value<T>();
}

template <typename T>
constexpr Vector2<T>::operator bool() const {
    return x || y;
}

template <typename T>
constexpr T Vector2<T>::squaredLength() const {
    return x*x+y*y;
}

template <typename T>
constexpr T Vector2<T>::length() const {
    return T(std::sqrt(x*x+y*y));
}

template <typename T>
constexpr Vector2<T> Vector2<T>::normalized(const Vector2<T> &zeroResult) const {
    if (T len = length())
        return *this/len;
    return zeroResult;
}

template <typename T>
constexpr Vector2<T> Vector2<T>::orthogonal(int polarity) const {
    return T(polarity)*Vector2<T>(-y, x);
}

template <typename T>
constexpr Vector2<T> Vector2<T>::orthonormal(int polarity, const Vector2<T> &zeroResult) const {
    return orthogonal(polarity).normalized(zeroResult);
}

}
