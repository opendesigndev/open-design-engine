
#include "LayerChangeApplication.h"

#include <ode-essentials.h>

namespace ode {

#define UPDATE_CHANGE_LEVEL(level) \
    if ((level) > changeLevel) \
        changeLevel = (level);

// TODO: Invalid index error ?
#define CHECK_INDEX(layerValues) \
    if (!layerChange.index.has_value() || layer.layerValues.size() <= *layerChange.index) { \
        return DesignError::UNKNOWN_ERROR; \
    }

// TODO: Invalid layer change value error ?
#define CHECK_ATTRIB(changeAttrib) \
    if (!layerChange.values.changeAttrib.has_value()) { \
        return DesignError::UNKNOWN_ERROR; \
    }

#define CHECK_CHANGE(layerValues, changeAttrib) \
    CHECK_ATTRIB(changeAttrib) \
    CHECK_INDEX(layerValues)
#define CHECK_INSERT(layerValues, changeAttrib) \
    CHECK_ATTRIB(changeAttrib) \
    if (layerChange.index.has_value() && layer.layerValues.size() <= *layerChange.index) { \
        return DesignError::UNKNOWN_ERROR; \
    }
#define CHECK_REMOVE(layerValues) \
    CHECK_INDEX(layerValues)

#define APPLY_PROP_CHANGE_LAYER(changeAttrib, level) \
    CHECK_ATTRIB(changeAttrib) \
    layer.changeAttrib = layerChange.values.changeAttrib.value(); \
    UPDATE_CHANGE_LEVEL(changeLevel)

#define APPLY_PROP_CHANGE(element, changeAttrib, level) \
    CHECK_ATTRIB(changeAttrib) \
    layer.element->changeAttrib = layerChange.values.changeAttrib.value(); \
    UPDATE_CHANGE_LEVEL(changeLevel)

#define APPLY_REPLACE(layerValues, changeAttrib, level) \
    CHECK_CHANGE(layerValues, changeAttrib) \
    layer.layerValues[*layerChange.index] = *layerChange.values.changeAttrib; \
    UPDATE_CHANGE_LEVEL(level)

#define APPLY_INSERT(layerValues, changeAttrib, level) \
    CHECK_INSERT(layerValues, changeAttrib) \
    layer.layerValues.insert(layerChange.index.has_value() ? layer.layerValues.begin()+*layerChange.index : layer.layerValues.end(), *layerChange.values.changeAttrib); \
    UPDATE_CHANGE_LEVEL(level)

#define APPLY_REMOVE(layerValues, level) \
    CHECK_REMOVE(layerValues); \
    layer.layerValues.erase(layer.layerValues.begin()+*layerChange.index); \
    UPDATE_CHANGE_LEVEL(level)


/*static*/ DesignError LayerChangeApplication::apply(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    switch (layerChange.subject) {
        case octopus::LayerChange::Subject::LAYER:
            return applyOnLayer(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::SHAPE:
            return applyOnShape(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::TEXT:
            return applyOnText(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::FILL:
            return applyOnFill(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::STROKE:
            return applyOnStroke(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::STROKE_FILL:
            return applyOnStrokeFill(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::EFFECT:
            return applyOnEffect(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::EFFECT_FILL:
            return applyOnEffectFill(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::FILL_FILTER:
            return applyOnFillFilter(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::STROKE_FILL_FILTER:
            return applyOnStrokeFillFilter(layerChange, layer, changeLevel);
        case octopus::LayerChange::Subject::EFFECT_FILL_FILTER:
            return applyOnEffectFillFilter(layerChange, layer, changeLevel);
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::NOT_IMPLEMENTED;
}

/*static*/ DesignError LayerChangeApplication::applyOnLayer(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::LAYER);

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            if (
                (layerChange.values.shape.has_value() && layer.type != octopus::Layer::Type::SHAPE) ||
                (layerChange.values.text.has_value() && layer.type != octopus::Layer::Type::TEXT) ||
                (layerChange.values.maskBasis.has_value() && layer.type != octopus::Layer::Type::MASK_GROUP) ||
                (layerChange.values.maskChannels.has_value() && layer.type != octopus::Layer::Type::MASK_GROUP) ||
                (layerChange.values.componentId.has_value() && layer.type != octopus::Layer::Type::COMPONENT_REFERENCE)
            )
                return DesignError::WRONG_LAYER_TYPE;
            if (layerChange.values.transform.has_value()) {
                if (layerChange.values.transform.value()[0]*layerChange.values.transform.value()[3] == layerChange.values.transform.value()[1]*layerChange.values.transform.value()[2])
                    return DesignError::SINGULAR_TRANSFORMATION;
                memcpy(layer.transform, layerChange.values.transform->data(), sizeof(layer.transform));
                changeLevel = BOUNDS_CHANGE;
            }
            APPLY_PROP_CHANGE_LAYER(name, LOGICAL_CHANGE);
            APPLY_PROP_CHANGE_LAYER(visible, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE_LAYER(opacity, COMPOSITION_CHANGE); // composition change if opacity changes between zero / non-zero
            APPLY_PROP_CHANGE_LAYER(blendMode, COMPOSITION_CHANGE); // composition change if blend mode PASS_THROUGH
            APPLY_PROP_CHANGE_LAYER(shape, BOUNDS_CHANGE);
            APPLY_PROP_CHANGE_LAYER(text, BOUNDS_CHANGE);
            APPLY_PROP_CHANGE_LAYER(maskBasis, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE_LAYER(maskChannels, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE_LAYER(componentId, HIERARCHY_CHANGE);
            APPLY_PROP_CHANGE_LAYER(effects, COMPOSITION_CHANGE);
            break;
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnShape(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::SHAPE);

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
                return DesignError::WRONG_LAYER_TYPE;
            }
            APPLY_PROP_CHANGE(shape, fillRule, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE(shape, path, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE(shape, fills, COMPOSITION_CHANGE);
            APPLY_PROP_CHANGE(shape, strokes, COMPOSITION_CHANGE);
            break;
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnText(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::TEXT);

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            if (layer.type != octopus::Layer::Type::TEXT || !layer.text.has_value()) {
                return DesignError::WRONG_LAYER_TYPE;
            }
            APPLY_PROP_CHANGE(text, value, BOUNDS_CHANGE);
            APPLY_PROP_CHANGE(text, defaultStyle, COMPOSITION_CHANGE);
            break;
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::FILL);

    if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
        return DesignError::WRONG_LAYER_TYPE;
    }
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return DesignError::NOT_IMPLEMENTED;
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(shape->fills, fill, VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(shape->fills, fill, VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(shape->fills, VISUAL_CHANGE);
            break;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnStroke(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE);

    if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
        return DesignError::WRONG_LAYER_TYPE;
    }
    std::vector<octopus::Shape::Stroke> &octopusStrokes = layer.shape->strokes;
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return DesignError::NOT_IMPLEMENTED;
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(shape->strokes, stroke, BOUNDS_CHANGE);
            if (layerChange.values.fillRule.has_value()) {
                octopusStrokes[*layerChange.index].fillRule = layerChange.values.fillRule;
            }
            if (layerChange.values.path.has_value()) {
                octopusStrokes[*layerChange.index].path = layerChange.values.path;
            }
            break;
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(shape->strokes, stroke, BOUNDS_CHANGE);
            break;
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(shape->strokes, BOUNDS_CHANGE);
            break;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnEffect(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT);

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return DesignError::NOT_IMPLEMENTED;
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(effects, effect, VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(effects, effect, VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(effects, VISUAL_CHANGE);
            break;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnStrokeFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL);

    if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
        return DesignError::WRONG_LAYER_TYPE;
    }
    std::vector<octopus::Shape::Stroke> &octopusStrokes = layer.shape->strokes;
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            CHECK_CHANGE(shape->strokes, fill);
            octopusStrokes[*layerChange.index].fill = *layerChange.values.fill;
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnEffectFill(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL);

    std::vector<octopus::Effect> &octopusEffects = layer.effects;
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            CHECK_CHANGE(effects, fill);
            octopusEffects[*layerChange.index].overlay = *layerChange.values.fill;
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        default:
            return DesignError::NOT_IMPLEMENTED;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::FILL_FILTER);

    if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
        return DesignError::WRONG_LAYER_TYPE;
    }
    CHECK_INDEX(shape->fills);
    std::vector<octopus::Fill> &octopusFills = layer.shape->fills;
    nonstd::optional<std::vector<octopus::Filter>> &octopusFillFilters = octopusFills[*layerChange.index].filters;

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return DesignError::NOT_IMPLEMENTED;
        case octopus::LayerChange::Op::REPLACE:
            CHECK_ATTRIB(filter);
            // TODO: Invalid filter index
            if (!octopusFillFilters.has_value() || octopusFillFilters->size() <= *layerChange.filterIndex) {
                return DesignError::UNKNOWN_ERROR;
            }
            (*octopusFillFilters)[*layerChange.filterIndex] = *layerChange.values.filter;
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::INSERT:
            CHECK_ATTRIB(filter);
            if (octopusFillFilters.has_value()) {
                octopusFillFilters->insert(layerChange.filterIndex.has_value() ? octopusFillFilters->begin()+*layerChange.filterIndex : octopusFillFilters->end(), *layerChange.values.filter);
            } else {
                octopusFillFilters = std::vector<octopus::Filter> {
                    *layerChange.values.filter
                };
            }
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::REMOVE:
            // TODO: Invalid filter index
            if (!octopusFillFilters.has_value() || octopusFillFilters->size() <= *layerChange.filterIndex) {
                return DesignError::UNKNOWN_ERROR;
            }
            octopusFillFilters->erase(octopusFillFilters->begin()+*layerChange.filterIndex);
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnStrokeFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL_FILTER);

    if (layer.type != octopus::Layer::Type::SHAPE || !layer.shape.has_value()) {
        return DesignError::WRONG_LAYER_TYPE;
    }
    CHECK_INDEX(shape->strokes);
    std::vector<octopus::Shape::Stroke> &octopusStrokes = layer.shape->strokes;
    nonstd::optional<std::vector<octopus::Filter>> &octopusStrokeFillFilters = octopusStrokes[*layerChange.index].fill.filters;

    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return DesignError::NOT_IMPLEMENTED;
        case octopus::LayerChange::Op::REPLACE:
            CHECK_ATTRIB(filter);
            // TODO: Invalid filter index
            if (!octopusStrokeFillFilters.has_value() || octopusStrokeFillFilters->size() <= *layerChange.filterIndex) {
                return DesignError::UNKNOWN_ERROR;
            }
            (*octopusStrokeFillFilters)[*layerChange.filterIndex] = *layerChange.values.filter;
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::INSERT:
            CHECK_ATTRIB(filter);
            if (octopusStrokeFillFilters.has_value()) {
                octopusStrokeFillFilters->insert(layerChange.filterIndex.has_value() ? octopusStrokeFillFilters->begin()+*layerChange.filterIndex : octopusStrokeFillFilters->end(), *layerChange.values.filter);
            } else {
                octopusStrokeFillFilters = std::vector<octopus::Filter> {
                    *layerChange.values.filter
                };
            }
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
        case octopus::LayerChange::Op::REMOVE:
            // TODO: Invalid filter index
            if (!octopusStrokeFillFilters.has_value() || octopusStrokeFillFilters->size() <= *layerChange.filterIndex) {
                return DesignError::UNKNOWN_ERROR;
            }
            octopusStrokeFillFilters->erase(octopusStrokeFillFilters->begin()+*layerChange.filterIndex);
            UPDATE_CHANGE_LEVEL(VISUAL_CHANGE);
            break;
    }
    return DesignError::OK;
}

/*static*/ DesignError LayerChangeApplication::applyOnEffectFillFilter(const octopus::LayerChange &layerChange, octopus::Layer &layer, ChangeLevel &changeLevel) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL_FILTER);

    // TODO: Effect fill filter
    std::vector<octopus::Effect> &octopusEffects = layer.effects;
    if (!layerChange.index.has_value() || *layerChange.index >= layer.effects.size()) {
        return DesignError::UNKNOWN_ERROR;
    }
    octopus::Effect &octopusEffect = octopusEffects[*layerChange.index];
    switch (octopusEffect.type) {
        case octopus::Effect::Type::OVERLAY:
            if (!octopusEffect.overlay.has_value()) {
                return DesignError::UNKNOWN_ERROR;
            }
            {
                // TODO:
            }
            break;
        case octopus::Effect::Type::STROKE:
            if (!octopusEffect.stroke.has_value()) {
                return DesignError::UNKNOWN_ERROR;
            }
            {
                // TODO:
            }
            break;
        case octopus::Effect::Type::DROP_SHADOW:
        case octopus::Effect::Type::INNER_SHADOW:
        case octopus::Effect::Type::OUTER_GLOW:
        case octopus::Effect::Type::INNER_GLOW:
            break;
        case octopus::Effect::Type::GAUSSIAN_BLUR:
        case octopus::Effect::Type::BOUNDED_BLUR:
        case octopus::Effect::Type::BLUR:
            if (!octopusEffect.filters.has_value()) {
                return DesignError::UNKNOWN_ERROR;
            }
            {
                // TODO:
            }
            break;
        case octopus::Effect::Type::OTHER:
            break;
    }

    return DesignError::OK;
}

} // namespace ode
