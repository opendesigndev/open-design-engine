
#pragma once

#include <octopus/effect.h>
#include "bounds.h"

namespace ode {

/// Computes the graphical margin of a given effect relative to the layer's bounds
UntransformedMargin effectMargin(const octopus::Effect &effect);

}
