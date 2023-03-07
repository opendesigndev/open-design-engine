
#pragma once

#include <octopus/effect.h>
#include "bounds.h"

namespace ode {

// sqrt(2)*erf^-1(1-2*minAlpha); minAlpha == 1/512.
constexpr double GAUSSIAN_BLUR_RANGE_FACTOR = 2.8856349124267571473876066463246112449484013782094917195589559;

/// Computes the graphical margin of a given effect relative to the layer's bounds
UntransformedMargin effectMargin(const octopus::Effect &effect);

}
