
#include "Rectangle.h"

namespace ode {

template <typename T, int VARIANT>
const Rectangle<T, VARIANT> Rectangle<T, VARIANT>::infinite(Vector2<T>(-T(~0u>>2)), Vector2<T>(T(~0u>>2)));

template <typename T, int VARIANT>
const Rectangle<T, VARIANT> Rectangle<T, VARIANT>::unspecified(Vector2<T>(T(~0u>>2)), Vector2<T>(-T(~0u>>2)));

template <typename T, int VARIANT>
constexpr Vector2<T> Rectangle<T, VARIANT>::dimensions() const {
    return b-a;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> Rectangle<T, VARIANT>::canonical() const {
    return operator bool() ? *this : Rectangle<T, VARIANT>();
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &Rectangle<T, VARIANT>::operator&=(const Rectangle<T, VARIANT> &other) {
    a.x = std::max(a.x, other.a.x);
    a.y = std::max(a.y, other.a.y);
    b.x = std::min(b.x, other.b.x);
    b.y = std::min(b.y, other.b.y);
    return *this;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &Rectangle<T, VARIANT>::operator|=(const Rectangle<T, VARIANT> &other) {
    a.x = std::min(a.x, other.a.x);
    a.y = std::min(a.y, other.a.y);
    b.x = std::max(b.x, other.b.x);
    b.y = std::max(b.y, other.b.y);
    return *this;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &Rectangle<T, VARIANT>::operator+=(const Vector2<T> &translate) {
    a += translate;
    b += translate;
    return *this;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &Rectangle<T, VARIANT>::operator-=(const Vector2<T> &translate) {
    a -= translate;
    b -= translate;
    return *this;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &Rectangle<T, VARIANT>::operator*=(T scale) {
    a *= scale;
    b *= scale;
    return *this;
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT>::operator bool() const {
    return a.x < b.x && a.y < b.y;
}

template <typename T, int VARIANT>
template <int DST_VARIANT>
constexpr Rectangle<T, VARIANT>::operator Rectangle<T, DST_VARIANT>() const {
    return Rectangle<T, DST_VARIANT>(a, b);
}

}
