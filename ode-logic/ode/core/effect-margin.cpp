
#include "effect-margin.h"

namespace ode {

UntransformedMargin effectMargin(const octopus::Effect &effect) {
    switch (effect.type) {
        case octopus::Effect::Type::OVERLAY:
        case octopus::Effect::Type::INNER_SHADOW:
        case octopus::Effect::Type::INNER_GLOW:
            return UntransformedMargin();
        case octopus::Effect::Type::STROKE:
            if (effect.stroke.has_value()) {
                switch (effect.stroke->position) {
                    case octopus::Stroke::Position::OUTSIDE:
                        return UntransformedMargin(effect.stroke->thickness);
                    case octopus::Stroke::Position::CENTER:
                        return UntransformedMargin(.5*effect.stroke->thickness);
                    case octopus::Stroke::Position::INSIDE:
                        return UntransformedMargin();
                }
            }
            break;
        case octopus::Effect::Type::DROP_SHADOW:
            if (effect.shadow.has_value()) {
                UntransformedMargin margin(effect.shadow->blur+effect.shadow->choke);
                margin.a.x -= effect.shadow->offset.x;
                margin.a.y -= effect.shadow->offset.y;
                margin.b.x += effect.shadow->offset.x;
                margin.b.y += effect.shadow->offset.y;
                return margin;
            }
            break;
        case octopus::Effect::Type::OUTER_GLOW:
            if (effect.glow.has_value())
                return UntransformedMargin(effect.glow->blur+effect.glow->choke);
            break;
        case octopus::Effect::Type::GAUSSIAN_BLUR:
        case octopus::Effect::Type::BLUR:
            if (effect.blur.has_value())
                return UntransformedMargin(GAUSSIAN_BLUR_RANGE_FACTOR*effect.blur.value());
            break;
        case octopus::Effect::Type::BOUNDED_BLUR:
            if (effect.blur.has_value())
                return UntransformedMargin(effect.blur.value());
            break;
        case octopus::Effect::Type::OTHER:
            // TODO warning
            break;
    }
    return UntransformedMargin();
}

}
