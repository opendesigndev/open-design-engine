
#pragma once

#include <algorithm>
#include "../math/Vector2.h"

namespace ode {

/// An axis-aligned rectangle specified by its opposite corners a, b (where a.x < b.x && a.y < b.y) (VARIANT prevents mixing of rectangles in different coordinate systems)
template <typename T, int VARIANT = 0>
struct Rectangle {

    /// a = first corner, b = last corner
    Vector2<T> a, b;

    /// An "infinite" rectangle, where (infinite & x) == x for any valid rectangle x
    static const Rectangle<T, VARIANT> infinite;
    /// A special empty rectangle value, such that (unspecified | x) == x for any valid rectangle x
    static const Rectangle<T, VARIANT> unspecified;

    constexpr Rectangle() { }
    constexpr Rectangle(T ax, T ay, T bx, T by) : a(ax, ay), b(bx, by) { }
    constexpr Rectangle(const Vector2<T> &a, const Vector2<T> &b) : a(a), b(b) { }

    constexpr Vector2<T> dimensions() const;
    /// Provides a unique representation of empty rectangles (all zeroes), otherwise returns self
    constexpr Rectangle<T, VARIANT> canonical() const;

    /// Intersection with other
    constexpr Rectangle<T, VARIANT> &operator&=(const Rectangle<T, VARIANT> &other);
    /// Expands rectangle to include other
    constexpr Rectangle<T, VARIANT> &operator|=(const Rectangle<T, VARIANT> &other);
    /// Adds translate vector to rectangle's corner points
    constexpr Rectangle<T, VARIANT> &operator+=(const Vector2<T> &translate);
    /// Subtracts translate vector to rectangle's corner points
    constexpr Rectangle<T, VARIANT> &operator-=(const Vector2<T> &translate);
    /// Scales rectangle by scale
    constexpr Rectangle<T, VARIANT> &operator*=(T scale);
    /// Returns true if rectangle has positive area
    constexpr explicit operator bool() const;
    template <int DST_VARIANT>
    constexpr explicit operator Rectangle<T, DST_VARIANT>() const;

    constexpr friend Rectangle<T, VARIANT> operator*(T scale, const Rectangle<T, VARIANT> &rectangle) {
        return Rectangle<T, VARIANT>(scale*rectangle.a, scale*rectangle.b);
    }

    constexpr friend Rectangle<T, VARIANT> operator*(const Rectangle<T, VARIANT> &rectangle, T scale) {
        return Rectangle<T, VARIANT>(rectangle.a*scale, rectangle.b*scale);
    }

};

/// Computes the intersection of two rectangles
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator&(const Rectangle<T, VARIANT> &a, const Rectangle<T, VARIANT> &b) {
    return Rectangle<T, VARIANT>(std::max(a.a.x, b.a.x), std::max(a.a.y, b.a.y), std::min(a.b.x, b.b.x), std::min(a.b.y, b.b.y));
}

/// Computes the bounding rectangle of two rectangles
template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator|(const Rectangle<T, VARIANT> &a, const Rectangle<T, VARIANT> &b) {
    return Rectangle<T, VARIANT>(std::min(a.a.x, b.a.x), std::min(a.a.y, b.a.y), std::max(a.b.x, b.b.x), std::max(a.b.y, b.b.y));
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator+(const Vector2<T> &translate, const Rectangle<T, VARIANT> &rectangle) {
    return Rectangle<T, VARIANT>(translate+rectangle.a, translate+rectangle.b);
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator+(const Rectangle<T, VARIANT> &rectangle, const Vector2<T> &translate) {
    return Rectangle<T, VARIANT>(rectangle.a+translate, rectangle.b+translate);
}

template <typename T, int VARIANT>
constexpr Rectangle<T, VARIANT> operator-(const Rectangle<T, VARIANT> &rectangle, const Vector2<T> &translate) {
    return Rectangle<T, VARIANT>(rectangle.a-translate, rectangle.b-translate);
}

template <typename T, int VARIANT>
constexpr bool operator==(const Rectangle<T, VARIANT> &a, const Rectangle<T, VARIANT> &b) {
    return a.a == b.a && a.b == b.b;
}

template <typename T, int VARIANT>
constexpr bool operator!=(const Rectangle<T, VARIANT> &a, const Rectangle<T, VARIANT> &b) {
    return a.a != b.a || a.b != b.b;
}

}

#include "Rectangle.hpp"
