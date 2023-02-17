
#pragma once

namespace ode {

/// Represents a color (color space may depend on context)
struct Color {
    /// Values of individual color channels, between 0 and 1
    double r, g, b, a;

    constexpr Color() : r(0), g(0), b(0), a(1) { }
    constexpr explicit Color(double rgb, double a = 1) : r(rgb), g(rgb), b(rgb), a(a) { }
    constexpr explicit Color(double r, double g, double b, double a = 1) : r(r), g(g), b(b), a(a) { }
};

}
