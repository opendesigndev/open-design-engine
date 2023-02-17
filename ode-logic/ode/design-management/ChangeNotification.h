
#pragma once

#include <string>

namespace ode {

enum class LayerChangeType {
    NO_CHANGE,
    RENAME, // name property
    VISIBILITY,
    OPACITY,
    BLEND_MODE,
    TRANSFORMATION, // or feature scale
    SHAPE_FILL, // or stroke fill - does not change bounds
    SHAPE_STROKE,
    SHAPE_GEOMETRY, // including stroke path geometry
    TEXT_FILL, // does not change bounds
    TEXT_CONTENT,
    MASK_BASIS,
    CHILD_LAYER_ADDED,
    CHILD_LAYER_REMOVED,
    // Note: modifications within child layers not reported this way
    EFFECT_ADDED,
    EFFECT_REMOVED,
    EFFECT_FILL, // a change to an effect that does not change bounds
    EFFECT,
    COMPONENT_REFERENCE,
    OVERRIDE_ADDED,
    OVERRIDE_REMOVED,
    OVERRIDE,
    OVERRIDE_DISABLEMENT
};

struct ComponentChange {
    std::string component;
    // If layer is empty, component's root has been replaced
    std::string layer;
    LayerChangeType changeType;
};

class ComponentChangeListener {
public:
    virtual ~ComponentChangeListener() = default;
    virtual void operator()(const ComponentChange &componentChange) = 0;
};

}
