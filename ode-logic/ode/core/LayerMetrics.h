
#pragma once

#include "TransformationMatrix.h"
#include "LayerBounds.h"

namespace ode {

struct LayerMetrics : LayerBounds {
    TransformationMatrix transformation;
    double featureScale;
};

}
