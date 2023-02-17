
#pragma once

#include "Rectangle.h"

namespace ode {

/// Specifies a margin on the four sides of any axis-aligned rectangle (VARIANT prevents mixing of rectangle margins in different coordinate systems)
template <typename T, int VARIANT = 0>
struct RectangleMargin {

    /// a = left and top margin, b = right and bottom margin
    Vector2<T> a, b;

    constexpr RectangleMargin() { }
    constexpr explicit RectangleMargin(T uniform) : a(uniform), b(uniform) { }
    constexpr RectangleMargin(T ax, T ay, T bx, T by) : a(ax, ay), b(bx, by) { }
    constexpr RectangleMargin(const Vector2<T> &a, const Vector2<T> &b) : a(a), b(b) { }

    constexpr RectangleMargin<T, VARIANT> &operator+=(const RectangleMargin<T, VARIANT> &other);
    constexpr RectangleMargin<T, VARIANT> &operator-=(const RectangleMargin<T, VARIANT> &other);
    /// On each side, sets the margin to the minimum of this and other
    constexpr RectangleMargin<T, VARIANT> &operator&=(const RectangleMargin<T, VARIANT> &other);
    /// On each side, sets the margin to the maximum of this and other
    constexpr RectangleMargin<T, VARIANT> &operator|=(const RectangleMargin<T, VARIANT> &other);
    /// Scales (multiplies) margin on all sides by scale
    constexpr RectangleMargin<T, VARIANT> &operator*=(T scale);
    constexpr const RectangleMargin<T, VARIANT> &operator+() const;
    constexpr RectangleMargin<T, VARIANT> operator-() const;
    constexpr explicit operator bool() const;

    constexpr friend RectangleMargin<T, VARIANT> operator*(T scale, const RectangleMargin<T, VARIANT> &margin) {
        return RectangleMargin<T, VARIANT>(scale*margin.a, scale*margin.b);
    }

    constexpr friend RectangleMargin<T, VARIANT> operator*(const RectangleMargin<T, VARIANT> &margin, T scale) {
        return RectangleMargin<T, VARIANT>(margin.a*scale, margin.b*scale);
    }

};

/// Returns rectangle with the margin applied
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator+(const Rectangle<T, VARIANT> &rectangle, const RectangleMargin<T, VARIANT> &margin) {
    return Rectangle<T, VARIANT>(rectangle.a-margin.a, rectangle.b+margin.b);
}

/// Returns rectangle with the inverse margin applied
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator-(const Rectangle<T, VARIANT> &rectangle, const RectangleMargin<T, VARIANT> &margin) {
    return Rectangle<T, VARIANT>(rectangle.a+margin.a, rectangle.b-margin.b);
}

/// Adds margin to rectangle
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &operator+=(Rectangle<T, VARIANT> &rectangle, const RectangleMargin<T, VARIANT> &margin) {
    rectangle.a -= margin.a;
    rectangle.b += margin.b;
    return rectangle;
}

/// Subtracts margin from rectangle
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> &operator-=(Rectangle<T, VARIANT> &rectangle, const RectangleMargin<T, VARIANT> &margin) {
    rectangle.a += margin.a;
    rectangle.b -= margin.b;
    return rectangle;
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> operator+(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return RectangleMargin<T, VARIANT>(a.a+b.a, a.b+b.b);
}

template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> operator-(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return RectangleMargin<T, VARIANT>(a.a-b.a, a.b-b.b);
}

/// Computes the minimum margin for each side
template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> operator&(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return RectangleMargin<T, VARIANT>(std::min(a.a.x, b.a.x), std::min(a.a.y, b.a.y), std::min(a.b.x, b.b.x), std::min(a.b.y, b.b.y));
}

/// Computes the maximum margin for each side
template <typename T, int VARIANT>
constexpr RectangleMargin<T, VARIANT> operator|(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return RectangleMargin<T, VARIANT>(std::max(a.a.x, b.a.x), std::max(a.a.y, b.a.y), std::max(a.b.x, b.b.x), std::max(a.b.y, b.b.y));
}

template <typename T, int VARIANT>
constexpr bool operator==(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return a.a == b.a && a.b == b.b;
}

template <typename T, int VARIANT>
constexpr bool operator!=(const RectangleMargin<T, VARIANT> &a, const RectangleMargin<T, VARIANT> &b) {
    return a.a != b.a || a.b != b.b;
}

}

#include "RectangleMargin.hpp"
