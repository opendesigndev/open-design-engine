
#pragma once

#include <nonstd/optional.hpp>
#include <octopus/octopus.h>
#include "../render-expressions/RenderExpression.h"
#include "Facets.hpp"

namespace ode {

class Facets {

public:
    /// Facet has been requested
    static constexpr int FLAG_REQUESTED = 0x01;
    /// Only the alpha channel needs to be rendered
    static constexpr int FLAG_ALPHA_ONLY = 0x02;
    /// Facet's render has been provided but is empty
    static constexpr int FLAG_NULL = 0x10;

    class Facet {
        int i;
    public:
        static constexpr int FACET_COUNT = 6;
        constexpr Facet(octopus::EffectBasis effectBasis) : i(facetIndex(effectBasis)) { }
        constexpr Facet(octopus::MaskBasis maskBasis) : i(facetIndex(maskBasis)) { }
        constexpr operator int() const { return i; }
    };

    static const Facet BODY;
    static const Facet BODY_AND_STROKES;
    static const Facet FILL;
    static const Facet LAYER_AND_EFFECTS;
    static const Facet BACKGROUND;

    //nonstd::optional<Rendexptr> &operator[](const Facet &facet);
    //const nonstd::optional<Rendexptr> &operator[](const Facet &facet) const;
    void request(const Facet &facet, int flags);
    int requested(const Facet &facet) const;
    bool finished() const;
    void set(const Facet &facet, const Rendexptr &render);
    Rendexptr get(const Facet &facet) const;

private:
    struct Entry {
        int flags = 0;
        Rendexptr render;
    };

    nonstd::optional<Entry> facets[Facet::FACET_COUNT];

};

}
