
#include "layer-change-apply.h"

#define REQUIRE_PROPERTY(prop) do { \
    if (!layerChange.values.prop.has_value()) \
        return DesignError::LAYER_CHANGE_MISSING_VALUE; \
} while (false)

#define UPDATE_CHANGE_LEVEL(level) do { \
    if (int(ChangeLevel::level) > int(changeLevel)) \
        changeLevel = ChangeLevel::level; \
} while (false)

#define APPLY_PROPERTY_CHANGE(prop, level) do { \
    if (layerChange.values.prop.has_value()) { \
        target.prop = layerChange.values.prop.value(); \
        UPDATE_CHANGE_LEVEL(level); \
    } \
} while (false)

#define APPLY_REPLACE(dst, prop, level) do { \
    REQUIRE_PROPERTY(prop); \
    (dst)[layerChange.index.value()] = layerChange.values.prop.value(); \
    return ChangeLevel::level; \
} while (false)

#define APPLY_INSERT(dst, prop, level) do { \
    REQUIRE_PROPERTY(prop); \
    (dst).insert(layerChange.index.has_value() ? (dst).begin()+layerChange.index.value() : (dst).end(), layerChange.values.prop.value()); \
    return ChangeLevel::level; \
} while (false)

#define APPLY_REMOVE(dst, level) do { \
    (dst).erase((dst).begin()+layerChange.index.value()); \
    return ChangeLevel::level; \
} while (false)

#define CHECK_INDEX(subject) do { \
    if (!(layerChange.op == octopus::LayerChange::Op::INSERT && !usesFilterIndex(layerChange) ? ( \
        (size_t) layerChange.index.value_or(0) <= (subject).size() \
    ) : ( \
        layerChange.index.has_value() && (size_t) layerChange.index.value() < (subject).size() \
    ))) \
        return DesignError::LAYER_CHANGE_INVALID_INDEX; \
} while (false)

namespace ode {

static bool usesFilterIndex(const octopus::LayerChange &layerChange) {
    return layerChange.subject == octopus::LayerChange::Subject::FILL_FILTER || layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL_FILTER || layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL_FILTER;
}

static Result<ChangeLevel, DesignError> applyOnFill(octopus::Fill &target, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(
        layerChange.subject == octopus::LayerChange::Subject::FILL ||
        layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL ||
        layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL
    );
    ChangeLevel changeLevel = ChangeLevel::NONE;
    if (layerChange.op == octopus::LayerChange::Op::PROPERTY_CHANGE) {
        APPLY_PROPERTY_CHANGE(visible, COMPOSITION);
        APPLY_PROPERTY_CHANGE(blendMode, COMPOSITION);
        APPLY_PROPERTY_CHANGE(filters, COMPOSITION);
    } else
        return DesignError::LAYER_CHANGE_INVALID_OP;
    return changeLevel;
}

static Result<ChangeLevel, DesignError> applyOnFilters(nonstd::optional<std::vector<octopus::Filter> > &filters, const octopus::LayerChange &layerChange) {
    if (!(layerChange.op == octopus::LayerChange::Op::INSERT ? (
        (size_t) layerChange.filterIndex.value_or(0) <= (filters.has_value() ? filters->size() : (size_t) 0)
    ) : (
        filters.has_value() && layerChange.filterIndex.has_value() && (size_t) layerChange.filterIndex.value() < filters->size()
    )))
        return DesignError::LAYER_CHANGE_INVALID_INDEX;
    ChangeLevel changeLevel = ChangeLevel::NONE;
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            {
                octopus::Filter &target = filters.value()[layerChange.filterIndex.value()];
                APPLY_PROPERTY_CHANGE(visible, COMPOSITION);
            }
            break;
        case octopus::LayerChange::Op::REPLACE:
            REQUIRE_PROPERTY(filter);
            filters.value()[layerChange.filterIndex.value()] = layerChange.values.filter.value();
            changeLevel = ChangeLevel::COMPOSITION; // composition change on visibility change
            break;
        case octopus::LayerChange::Op::INSERT:
            REQUIRE_PROPERTY(filter);
            if (!filters.has_value())
                filters = std::vector<octopus::Filter>();
            filters->insert(layerChange.filterIndex.has_value() ? filters->begin()+layerChange.filterIndex.value() : filters->end(), layerChange.values.filter.value());
            changeLevel = ChangeLevel::COMPOSITION;
            break;
        case octopus::LayerChange::Op::REMOVE:
            filters->erase(filters->begin()+layerChange.filterIndex.value());
            changeLevel = ChangeLevel::COMPOSITION;
            break;
    }
    return changeLevel;
}

static Result<ChangeLevel, DesignError> applyOnLayer(octopus::Layer &target, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::LAYER);
    ChangeLevel changeLevel = ChangeLevel::NONE;
    if (layerChange.op == octopus::LayerChange::Op::PROPERTY_CHANGE) {
        if (
            (layerChange.values.shape.has_value() && target.type != octopus::Layer::Type::SHAPE) ||
            (layerChange.values.text.has_value() && target.type != octopus::Layer::Type::TEXT) ||
            (layerChange.values.maskBasis.has_value() && target.type != octopus::Layer::Type::MASK_GROUP) ||
            (layerChange.values.maskChannels.has_value() && target.type != octopus::Layer::Type::MASK_GROUP) ||
            (layerChange.values.componentId.has_value() && target.type != octopus::Layer::Type::COMPONENT_REFERENCE)
        )
            return DesignError::WRONG_LAYER_TYPE;
        if (layerChange.values.transform.has_value()) {
            if (layerChange.values.transform.value()[0]*layerChange.values.transform.value()[3] == layerChange.values.transform.value()[1]*layerChange.values.transform.value()[2])
                return DesignError::SINGULAR_TRANSFORMATION;
            memcpy(target.transform, layerChange.values.transform->data(), sizeof(target.transform));
            UPDATE_CHANGE_LEVEL(BOUNDS);
        }
        APPLY_PROPERTY_CHANGE(name, LOGICAL);
        APPLY_PROPERTY_CHANGE(visible, COMPOSITION);
        APPLY_PROPERTY_CHANGE(opacity, COMPOSITION); // composition change if opacity changes between zero / non-zero
        APPLY_PROPERTY_CHANGE(blendMode, COMPOSITION); // composition change if blendMode changes to/from PASS_THROUGH
        APPLY_PROPERTY_CHANGE(shape, BOUNDS);
        APPLY_PROPERTY_CHANGE(text, BOUNDS);
        APPLY_PROPERTY_CHANGE(maskBasis, COMPOSITION);
        APPLY_PROPERTY_CHANGE(maskChannels, COMPOSITION);
        APPLY_PROPERTY_CHANGE(componentId, HIERARCHY);
        APPLY_PROPERTY_CHANGE(effects, COMPOSITION);
    } else
        return DesignError::LAYER_CHANGE_INVALID_OP;
    return changeLevel;
}

static Result<ChangeLevel, DesignError> applyOnShape(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::SHAPE);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    ChangeLevel changeLevel = ChangeLevel::NONE;
    if (layerChange.op == octopus::LayerChange::Op::PROPERTY_CHANGE) {
        octopus::Shape &target = layer.shape.value();
        APPLY_PROPERTY_CHANGE(fillRule, VISUAL);
        APPLY_PROPERTY_CHANGE(path, BOUNDS);
        APPLY_PROPERTY_CHANGE(fills, COMPOSITION);
        APPLY_PROPERTY_CHANGE(strokes, COMPOSITION);
    } else
        return DesignError::LAYER_CHANGE_INVALID_OP;
    return changeLevel;
}

static Result<ChangeLevel, DesignError> applyOnText(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::TEXT);
    if (!(layer.type == octopus::Layer::Type::TEXT && layer.text.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    ChangeLevel changeLevel = ChangeLevel::NONE;
    if (layerChange.op == octopus::LayerChange::Op::PROPERTY_CHANGE) {
        octopus::Text &target = layer.text.value();
        if (layerChange.values.transform.has_value()) {
            if (layerChange.values.transform.value()[0]*layerChange.values.transform.value()[3] == layerChange.values.transform.value()[1]*layerChange.values.transform.value()[2])
                return DesignError::SINGULAR_TRANSFORMATION;
            memcpy(target.transform, layerChange.values.transform->data(), sizeof(layer.transform));
            UPDATE_CHANGE_LEVEL(BOUNDS);
        }
        APPLY_PROPERTY_CHANGE(value, BOUNDS);
        APPLY_PROPERTY_CHANGE(defaultStyle, BOUNDS);
        APPLY_PROPERTY_CHANGE(styles, BOUNDS);
    } else
        return DesignError::LAYER_CHANGE_INVALID_OP;
    return changeLevel;
}

static Result<ChangeLevel, DesignError> applyOnFills(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::FILL);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    CHECK_INDEX(layer.shape->fills);
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            return applyOnFill(layer.shape->fills[layerChange.index.value()], layerChange);
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(layer.shape->fills, fill, COMPOSITION); // composition change if replacement fill has different filters
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(layer.shape->fills, fill, COMPOSITION);
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(layer.shape->fills, COMPOSITION);
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

static Result<ChangeLevel, DesignError> applyOnStrokes(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    CHECK_INDEX(layer.shape->strokes);
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            {
                octopus::Shape::Stroke &target = layer.shape->strokes[layerChange.index.value()];
                ChangeLevel changeLevel = ChangeLevel::NONE;
                APPLY_PROPERTY_CHANGE(visible, COMPOSITION);
                APPLY_PROPERTY_CHANGE(fill, COMPOSITION);
                return changeLevel;
            }
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(layer.shape->strokes, stroke, COMPOSITION); // composition change if replacement stroke fill has different filters
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(layer.shape->strokes, stroke, COMPOSITION);
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(layer.shape->strokes, COMPOSITION);
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

static Result<ChangeLevel, DesignError> applyOnStrokeFill(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    CHECK_INDEX(layer.shape->strokes);
    return applyOnFill(layer.shape->strokes[layerChange.index.value()].fill, layerChange);
}

static Result<ChangeLevel, DesignError> applyOnEffects(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT);
    CHECK_INDEX(layer.effects);
    switch (layerChange.op) {
        case octopus::LayerChange::Op::PROPERTY_CHANGE:
            {
                octopus::Effect &target = layer.effects[layerChange.index.value()];
                if (layerChange.values.filters.has_value() && target.type != octopus::Effect::Type::GAUSSIAN_BLUR && target.type != octopus::Effect::Type::BOUNDED_BLUR && target.type != octopus::Effect::Type::BLUR)
                    return DesignError::LAYER_CHANGE_INVALID_SUBJECT;
                ChangeLevel changeLevel = ChangeLevel::NONE;
                APPLY_PROPERTY_CHANGE(visible, COMPOSITION);
                APPLY_PROPERTY_CHANGE(blendMode, COMPOSITION);
                APPLY_PROPERTY_CHANGE(basis, COMPOSITION);
                APPLY_PROPERTY_CHANGE(filters, COMPOSITION);
                return changeLevel;
            }
        case octopus::LayerChange::Op::REPLACE:
            APPLY_REPLACE(layer.effects, effect, VISUAL);
        case octopus::LayerChange::Op::INSERT:
            APPLY_INSERT(layer.effects, effect, VISUAL);
        case octopus::LayerChange::Op::REMOVE:
            APPLY_REMOVE(layer.effects, VISUAL);
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

static Result<ChangeLevel, DesignError> applyOnEffectFill(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL);
    CHECK_INDEX(layer.effects);
    octopus::Effect &effect = layer.effects[layerChange.index.value()];
    switch (effect.type) {
        case octopus::Effect::Type::OVERLAY:
            if (!effect.overlay.has_value())
                return DesignError::INVALID_COMPONENT;
            return applyOnFill(effect.overlay.value(), layerChange);
        case octopus::Effect::Type::STROKE:
            if (!effect.stroke.has_value())
                return DesignError::INVALID_COMPONENT;
            return applyOnFill(effect.stroke->fill, layerChange);
        case octopus::Effect::Type::GAUSSIAN_BLUR:
        case octopus::Effect::Type::BOUNDED_BLUR:
        case octopus::Effect::Type::BLUR:
        case octopus::Effect::Type::DROP_SHADOW:
        case octopus::Effect::Type::INNER_SHADOW:
        case octopus::Effect::Type::OUTER_GLOW:
        case octopus::Effect::Type::INNER_GLOW:
        case octopus::Effect::Type::OTHER:
            return DesignError::LAYER_CHANGE_INVALID_SUBJECT;
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

static Result<ChangeLevel, DesignError> applyOnFillFilters(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::FILL_FILTER);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    CHECK_INDEX(layer.shape->fills);
    return applyOnFilters(layer.shape->fills[layerChange.index.value()].filters, layerChange);
}

static Result<ChangeLevel, DesignError> applyOnStrokeFillFilters(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::STROKE_FILL_FILTER);
    if (!(layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()))
        return DesignError::WRONG_LAYER_TYPE;
    CHECK_INDEX(layer.shape->strokes);
    return applyOnFilters(layer.shape->strokes[layerChange.index.value()].fill.filters, layerChange);
}

static Result<ChangeLevel, DesignError> applyOnEffectFillFilters(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    ODE_ASSERT(layerChange.subject == octopus::LayerChange::Subject::EFFECT_FILL_FILTER);
    CHECK_INDEX(layer.effects);
    octopus::Effect &effect = layer.effects[layerChange.index.value()];
    switch (effect.type) {
        case octopus::Effect::Type::OVERLAY:
            if (!effect.overlay.has_value())
                return DesignError::INVALID_COMPONENT;
            return applyOnFilters(effect.overlay->filters, layerChange);
        case octopus::Effect::Type::STROKE:
            if (!effect.stroke.has_value())
                return DesignError::INVALID_COMPONENT;
            return applyOnFilters(effect.stroke->fill.filters, layerChange);
        case octopus::Effect::Type::GAUSSIAN_BLUR:
        case octopus::Effect::Type::BOUNDED_BLUR:
        case octopus::Effect::Type::BLUR:
            //return applyOnFilters(effect.filters, layerChange); - not a "fill filter"
        case octopus::Effect::Type::DROP_SHADOW:
        case octopus::Effect::Type::INNER_SHADOW:
        case octopus::Effect::Type::OUTER_GLOW:
        case octopus::Effect::Type::INNER_GLOW:
        case octopus::Effect::Type::OTHER:
            return DesignError::LAYER_CHANGE_INVALID_SUBJECT;
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

Result<ChangeLevel, DesignError> applyLayerChange(octopus::Layer &layer, const octopus::LayerChange &layerChange) {
    switch (layerChange.subject) {
        case octopus::LayerChange::Subject::LAYER:
            return applyOnLayer(layer, layerChange);
        case octopus::LayerChange::Subject::SHAPE:
            return applyOnShape(layer, layerChange);
        case octopus::LayerChange::Subject::TEXT:
            return applyOnText(layer, layerChange);
        case octopus::LayerChange::Subject::FILL:
            return applyOnFills(layer, layerChange);
        case octopus::LayerChange::Subject::STROKE:
            return applyOnStrokes(layer, layerChange);
        case octopus::LayerChange::Subject::STROKE_FILL:
            return applyOnStrokeFill(layer, layerChange);
        case octopus::LayerChange::Subject::EFFECT:
            return applyOnEffects(layer, layerChange);
        case octopus::LayerChange::Subject::EFFECT_FILL:
            return applyOnEffectFill(layer, layerChange);
        case octopus::LayerChange::Subject::FILL_FILTER:
            return applyOnFillFilters(layer, layerChange);
        case octopus::LayerChange::Subject::STROKE_FILL_FILTER:
            return applyOnStrokeFillFilters(layer, layerChange);
        case octopus::LayerChange::Subject::EFFECT_FILL_FILTER:
            return applyOnEffectFillFilters(layer, layerChange);
    }
    ODE_ASSERT(!"Enum switch outdated!");
    return DesignError::NOT_IMPLEMENTED;
}

}
