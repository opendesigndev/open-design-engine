
#pragma once

#include <ode/graphics/Color.h>
#include "../core/TransformationMatrix.h"
#include "DocumentAnimation.h"

namespace ode {

/// Computes the transformation matrix in effect at time for a given LayerAnimation of type TRANSFORM or ROTATION
TransformationMatrix animateTransform(const LayerAnimation &animation, double time);
/// Computes the rotation angle in effect at time for a given LayerAnimation of type ROTATION
double animateRotation(const LayerAnimation &animation, double time);
/// Computes the opacity multiplier in effect at time for a given LayerAnimation of type OPACITY
double animateOpacity(const LayerAnimation &animation, double time);
/// Computes the color modifier in effect at time for a given LayerAnimation of type FILL_COLOR
Color animateColor(const LayerAnimation &animation, double time);

LayerAnimation::Keyframe animate(const LayerAnimation &animation, double time);

}
