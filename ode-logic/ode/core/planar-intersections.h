
#pragma once

#include <ode-essentials.h>
#include "TransformationMatrix.h"
#include "bounds.h"

namespace ode {

bool intersects(const UntransformedBounds &bounds, const TransformationMatrix &transformation, const Vector2d &center, double radius);

}
