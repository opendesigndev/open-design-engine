
#pragma once

#include "DesignError.h"

namespace ode {

/// Helper class to apply a LayerChange to layers
class LayerChangeApplication {
public:
    /// Change level
    enum ChangeLevel {
        NO_CHANGE,
        LOGICAL_CHANGE, // non-visual changes like layer name
        VISUAL_CHANGE, // visual change with no side effects (e.g. color change)
        BOUNDS_CHANGE, // change that affects bounds of layer(s)
        COMPOSITION_CHANGE, // change that affects generated render expression tree
        HIERARCHY_CHANGE // changes the layer hierarchy or component linkage
    } changeLevel = NO_CHANGE;

    /// Apply the LayerChange object to the specified layer
    static DesignError apply(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);

private:
    static DesignError applyOnLayer(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnShape(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnText(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnStroke(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnEffect(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnStrokeFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnEffectFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnStrokeFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);
    static DesignError applyOnEffectFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel);

    static DesignError applyFilters(const octopus::LayerChange &layerChange, nonstd::optional<std::vector<octopus::Filter>> &filters, ChangeLevel &changeLevel);
};

} // namespace ode
