
#pragma once

#include "bounds.h"

namespace ode {

struct LayerBounds {
    UntransformedBounds logicalBounds;
    // visual
    UntransformedBounds untransformedBounds;
    UnscaledBounds bounds;
    //UnscaledBounds fullBounds;
    //UnscaledBounds affectedBounds;
};

}
