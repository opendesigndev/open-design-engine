
#pragma once

#include <ode/geometry/Rectangle.h>
#include <ode/geometry/RectangleMargin.h>

namespace ode {

/// Integer bounds in output pixel coordinate system (corresponds to ScaledBounds)
typedef Rectangle<int, 1> PixelBounds;
/// Real bounds in output (transformed and scaled) coordinate system
typedef Rectangle<double, 1> ScaledBounds;
/// Real bounds in the unscaled (scale-agnostic) output coordinate system
typedef Rectangle<double, 2> UnscaledBounds;
/// Real bounds in the (untransformed) layer coordinate system
typedef Rectangle<double, 3> UntransformedBounds;

/// Integer pixel margin
typedef RectangleMargin<int, 1> PixelMargin;
/// Real margin in output coordinate system
typedef RectangleMargin<double, 1> ScaledMargin;
/// Real margin in unscaled output coordinate system
typedef RectangleMargin<double, 2> UnscaledMargin;
/// Real margin in layer coordinate system
typedef RectangleMargin<double, 3> UntransformedMargin;

}
