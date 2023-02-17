
#pragma once

#include <octopus/octopus.h>
#include "../core/LayerBounds.h"
#include "../core/LayerInstanceSpecifier.h"
#include "../render-expressions/RenderExpression.h"
#include "Facets.h"

namespace ode {

constexpr int ASSEMBLY_FLAG_FIXED_OPACITY = 0x20; // No layer opacity animation present

struct RendexSubtree {
    Rendexptr root;
    /// Inlays may be inserted at inlayPoint by placing new nodes into *inlayPoint and chaining its original value onto the new nodes
    Rendexptr *inlayPoint = nullptr; // NOTE ONLY BODY_MINUS_STROKES USES INLAYS, otherwise put on top of layer
    /// Layers to be blended below the subtree can be inserted at passthroughPoint in the same manner
    //Rendexptr *passthroughPoint = nullptr;
    // NOTE: If inlayPoint or passthroughPoint is null, that means pointer to root is the respective insertion point
    Rendexptr maskFacet;
};

void requestFacets(const LayerInstanceSpecifier &layer, Facets &facets, const nonstd::optional<octopus::MaskBasis> &maskBasis);

void assembleBackgroundLayerEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr &underlay);
void assembleForegroundLayerEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr &underlay, Rendexptr &overlay, Rendexptr *&inlayPoint);
RendexSubtree finalizeLayerAssembly(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, const Rendexptr &underlay, Rendexptr overlay, const Rendexptr &overlayMask, Rendexptr *inlayPoint, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags);

// NOTE: if basis is fill and layer type is text, remove output's alpha
Rendexptr applyFillReplacementEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr fill);
Rendexptr applyFilters(const nonstd::optional<std::vector<octopus::Filter> > &filters, Rendexptr fill);

RendexSubtree assembleShapeLayer(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags);
RendexSubtree assembleTextLayer(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags);

class GroupLayerAssembler {

public:
    GroupLayerAssembler(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags);
    GroupLayerAssembler(LayerInstanceSpecifier &&layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis) = delete;
    void setMask(const LayerInstanceSpecifier &layer, const RendexSubtree &subtree);
    void addLayer(const RendexSubtree &subtree);
    RendexSubtree get();

private:
    const LayerInstanceSpecifier &layer;
    const LayerBounds &bounds;
    const nonstd::optional<octopus::MaskBasis> &groupMaskBasis;
    RendexSubtree maskSubtree;
    nonstd::optional<octopus::MaskBasis> maskBasis;
    bool maskVisible = false;
    int flags;
    Rendexptr content;

};

}
