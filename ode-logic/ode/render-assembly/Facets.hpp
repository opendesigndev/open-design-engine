
#include <octopus/octopus.h>

namespace ode {

constexpr int facetIndex(octopus::EffectBasis effectBasis) {
    switch (effectBasis) {
        case octopus::EffectBasis::BODY:
            return 1;
        case octopus::EffectBasis::BODY_AND_STROKES:
            return 2;
        case octopus::EffectBasis::FILL:
            return 3;
        case octopus::EffectBasis::LAYER_AND_EFFECTS:
            return 4;
        case octopus::EffectBasis::BACKGROUND:
            return 5;
    }
    return 0;
}

constexpr int facetIndex(octopus::MaskBasis maskBasis) {
    switch (maskBasis) {
        case octopus::MaskBasis::SOLID:
            return 0;
        case octopus::MaskBasis::BODY:
        case octopus::MaskBasis::BODY_EMBED:
            return 1;
        case octopus::MaskBasis::FILL:
        case octopus::MaskBasis::FILL_EMBED:
            return 3;
        case octopus::MaskBasis::LAYER_AND_EFFECTS:
            return 4;
    }
    return 0;
}

}
