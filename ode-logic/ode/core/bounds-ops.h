
#pragma once

#include "bounds.h"
#include "TransformationMatrix.h"

namespace ode {

/// Transforms bounds by transformation matrix
UnscaledBounds transformBounds(const UntransformedBounds &bounds, const TransformationMatrix &matrix);
/// Scales unscaled bounds by scale
ScaledBounds scaleBounds(const UnscaledBounds &bounds, double scale);

/// Rounds to integer bounds such that partially covered pixels are included
PixelBounds outerPixelBounds(const ScaledBounds &bounds);
/// Rounds to integer bounds such that partially covered pixels are excluded
PixelBounds innerPixelBounds(const ScaledBounds &bounds);

}
