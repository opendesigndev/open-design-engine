
#include "Facets.h"

namespace ode {

const Facets::Facet Facets::BODY(octopus::EffectBasis::BODY);
const Facets::Facet Facets::BODY_AND_STROKES(octopus::EffectBasis::BODY_AND_STROKES);
const Facets::Facet Facets::FILL(octopus::EffectBasis::FILL);
const Facets::Facet Facets::LAYER_AND_EFFECTS(octopus::EffectBasis::LAYER_AND_EFFECTS);
const Facets::Facet Facets::BACKGROUND(octopus::EffectBasis::BACKGROUND);

void Facets::request(const Facet &facet, int flags) {
    if (!facets[facet].has_value())
        facets[facet] = Entry();
}

int Facets::requested(const Facet &facet) const {
    return facets[facet].has_value();
}

bool Facets::finished() const {
    for (const nonstd::optional<Entry> &entry : facets) {
        if (entry.has_value() && !entry->render)
            return false;
    }
    return true;
}

void Facets::set(const Facet &facet, const Rendexptr &render) {
    facets[facet] = Entry { 0, render };
}

Rendexptr Facets::get(const Facet &facet) const {
    return facets[facet].has_value() ? facets[facet]->render : nullptr;
}

}
