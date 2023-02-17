
#include "RectangleMargin.h"

namespace ode {

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator+=(const RectangleMargin<T, VARIANT> &other) {
    a += other.a;
    b += other.b;
    return *this;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator-=(const RectangleMargin<T, VARIANT> &other) {
    a -= other.a;
    b -= other.b;
    return *this;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator&=(const RectangleMargin<T, VARIANT> &other) {
    a.x = std::min(a.x, other.a.x);
    a.y = std::min(a.y, other.a.y);
    b.x = std::min(b.x, other.b.x);
    b.y = std::min(b.y, other.b.y);
    return *this;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator|=(const RectangleMargin<T, VARIANT> &other) {
    a.x = std::max(a.x, other.a.x);
    a.y = std::max(a.y, other.a.y);
    b.x = std::max(b.x, other.b.x);
    b.y = std::max(b.y, other.b.y);
    return *this;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator*=(T scale) {
    a *= scale;
    b *= scale;
    return *this;
}

template <typename T, int VARIANT>
constexpr const RectangleMargin<T, VARIANT> &RectangleMargin<T, VARIANT>::operator+() const {
    return *this;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> RectangleMargin<T, VARIANT>::operator-() const {
    return RectangleMargin<T, VARIANT>(-a, -b);
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT>::operator bool() const {
    return a && b;
}

}
