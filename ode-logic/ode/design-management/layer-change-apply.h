
#pragma once

#include <octopus/octopus.h>
#include <ode-essentials.h>
#include "DesignError.h"

namespace ode {

enum class ChangeLevel {
    NONE = 0,
    LOGICAL, // non-visual changes like layer name
    VISUAL, // visual change with no side effects (e.g. color change)
    BOUNDS, // change that affects bounds of layer(s)
    COMPOSITION, // change that affects generated render expression tree
    HIERARCHY // changes the layer hierarchy or component linkage
};

/// Apply the LayerChange object to the specified layer
Result<ChangeLevel, DesignError> applyLayerChange(octopus::Layer &layer, const octopus::LayerChange &layerChange);

}
