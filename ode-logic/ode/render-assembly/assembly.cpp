
#include "assembly.h"

#include "../render-expressions/render-operations.h"

namespace ode {

static bool isMaskActive(const nonstd::optional<octopus::MaskBasis> &maskBasis) {
    return maskBasis.value_or(octopus::MaskBasis::SOLID) != octopus::MaskBasis::SOLID;
}

static bool embedInMask(const nonstd::optional<octopus::MaskBasis> &maskBasis) {
    return maskBasis.has_value() && (maskBasis.value() == octopus::MaskBasis::BODY_EMBED || maskBasis.value() == octopus::MaskBasis::FILL_EMBED);
}

static bool isBlur(octopus::Effect::Type effectType) {
    return effectType == octopus::Effect::Type::GAUSSIAN_BLUR || effectType == octopus::Effect::Type::BOUNDED_BLUR || effectType == octopus::Effect::Type::BLUR;
}

static ChannelMatrix getChannelMatrix(const nonstd::optional<std::array<double, 5> > &octopusMaskChannels) {
    if (octopusMaskChannels.has_value()) {
        return ChannelMatrix { {
            octopusMaskChannels.value()[0],
            octopusMaskChannels.value()[1],
            octopusMaskChannels.value()[2],
            octopusMaskChannels.value()[3],
            octopusMaskChannels.value()[4]
        } };
    }
    return ChannelMatrix { { 0, 0, 0, 1, 0 } };
}

static octopus::BlendMode combineBlendMode(octopus::BlendMode outer, octopus::BlendMode inner) {
    if (outer == octopus::BlendMode::PASS_THROUGH) {
        if (inner == octopus::BlendMode::PASS_THROUGH) {
            // TODO REPORT AS WARNING
            return octopus::BlendMode::NORMAL;
        }
        return inner;
    }
    return outer;
}

static Rendexptr layerBackground(const LayerInstanceSpecifier &layer) {
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible && isBlur(effect.type) && effect.basis == octopus::EffectBasis::LAYER_AND_EFFECTS)
            return Rendexptr();
    }
    return makeBackground();
}

void requestFacets(const LayerInstanceSpecifier &layer, Facets &facets, const nonstd::optional<octopus::MaskBasis> &maskBasis) {
    if (isMaskActive(maskBasis))
        facets.request(maskBasis.value(), 0);
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible) {
            facets.request(effect.basis, 0);
        }
    }
}

void assembleBackgroundLayerEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr &underlay) {
    return;

    // TODO: ONLY BACKGROUND LAYER EFFECT IS DROP_SHADOW WITH "Layer Knocks Out Drop Shadow" DISABLED - currently not in Octopus

    int i = 0;
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible && effect.type == octopus::Effect::Type::DROP_SHADOW && effect.basis != octopus::EffectBasis::LAYER_AND_EFFECTS)
            underlay = blend(underlay, drawLayerEffect(facets.get(effect.basis), layer, i), effect.blendMode);
        ++i;
    }
}

void assembleForegroundLayerEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr &underlay, Rendexptr &overlay, Rendexptr *&inlayPoint) {
    int i = 0;
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible && effect.basis != octopus::EffectBasis::LAYER_AND_EFFECTS) {
            switch (effect.type) {
                case octopus::Effect::Type::OVERLAY:
                    if (effect.overlay.has_value() && effect.overlay->visible)
                        overlay = blend(overlay, applyFilters(effect.overlay.value().filters, drawLayerEffect(nullptr, layer, i)), combineBlendMode(effect.blendMode, effect.overlay->blendMode), inlayPoint);
                    break;
                case octopus::Effect::Type::STROKE:
                    if (effect.stroke.has_value() && effect.stroke->fill.visible) {
                        Rendexptr strokeRender = applyFilters(effect.stroke->fill.filters, drawLayerEffect(facets.get(effect.basis), layer, i));
                        octopus::BlendMode blendMode = combineBlendMode(effect.blendMode, effect.stroke->fill.blendMode);
                        // Possible minor optimization: if position == CENTER && underlay == overlay then blend to underlay, overlay = underlay
                        if (effect.stroke->position != octopus::Stroke::Position::INSIDE)
                            underlay = blend(underlay, strokeRender, blendMode);
                        if (effect.stroke->position != octopus::Stroke::Position::OUTSIDE)
                            overlay = blend(overlay, strokeRender, blendMode, inlayPoint);
                    }
                    break;
                case octopus::Effect::Type::DROP_SHADOW:
                case octopus::Effect::Type::OUTER_GLOW:
                    underlay = blend(underlay, drawLayerEffect(facets.get(effect.basis), layer, i), effect.blendMode);
                    break;
                case octopus::Effect::Type::INNER_SHADOW:
                case octopus::Effect::Type::INNER_GLOW:
                    overlay = blend(overlay, drawLayerEffect(facets.get(effect.basis), layer, i), effect.blendMode, inlayPoint);
                    break;
                case octopus::Effect::Type::GAUSSIAN_BLUR:
                case octopus::Effect::Type::BOUNDED_BLUR:
                case octopus::Effect::Type::BLUR:
                    // Resolved elsewhere
                case octopus::Effect::Type::OTHER:
                    // Unsupported (TODO warning)
                    break;
            }
        }
        ++i;
    }
}

RendexSubtree finalizeLayerAssembly(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, const Rendexptr &underlay, Rendexptr overlay, const Rendexptr &overlayMask, Rendexptr *inlayPoint, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags) {
    // Ensure that inlayPoint is available if it is needed (for BODY_MINUS_STROKES mask)
    if (embedInMask(maskBasis))
        overlay = makeInlayPoint(overlay, inlayPoint);
    Rendexptr render = mixMask(underlay, overlay, overlayMask, ChannelMatrix { { 0, 0, 0, 1, 0 } });

    Rendexptr layerAndEffects;
    int i = 0;
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible && isBlur(effect.type) && effect.basis == octopus::EffectBasis::LAYER_AND_EFFECTS) {
            /*
             * In the presence of this type of effect, the composition of the whole layer will be different.
             * Its entire likeness including effects will be combined first and then blended with the background using layer's blendMode.
             * Background is guaranteed to be absent by the function layerBackground
             */
            if (!layerAndEffects)
                layerAndEffects = render;
            layerAndEffects = applyFilters(effect.filters, drawLayerEffect(layerAndEffects, layer, i));
        }
        ++i;
    }
    if (layerAndEffects) {
        facets.set(Facets::LAYER_AND_EFFECTS, layerAndEffects);
        render = blend(makeBackground(), layerAndEffects, layer->blendMode);
    } else if (facets.requested(Facets::LAYER_AND_EFFECTS))
        facets.set(Facets::LAYER_AND_EFFECTS, unsetBackground(render));

    RendexSubtree subtree;
    if (flags&ASSEMBLY_FLAG_FIXED_OPACITY)
        subtree.root = mix(makeBackground(), render, layer->opacity);
    else
        subtree.root = mixLayerOpacity(layer, makeBackground(), render);
    subtree.inlayPoint = inlayPoint;
    if (isMaskActive(maskBasis))
        subtree.maskFacet = facets.get(maskBasis.value());
    return subtree;
}

Rendexptr applyFillReplacementEffects(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, Facets &facets, Rendexptr fill) {
    int i = 0;
    for (const octopus::Effect &effect : layer->effects) {
        if (effect.visible && isBlur(effect.type)) {
            switch (effect.basis) {
                case octopus::EffectBasis::FILL:
                    fill = applyFilters(effect.filters, drawLayerEffect(fill, layer, i));
                    break;
                case octopus::EffectBasis::BACKGROUND:
                    fill = applyFilters(effect.filters, drawLayerEffect(makeBackground(), layer, i));
                    break;
                case octopus::EffectBasis::LAYER_AND_EFFECTS:
                    // Resolved elsewhere
                    break;
                case octopus::EffectBasis::BODY:
                case octopus::EffectBasis::BODY_AND_STROKES:
                    // Unsupported (TODO warning)
                    break;
            }
        }
        ++i;
    }
    return fill;
}

Rendexptr applyFilters(const nonstd::optional<std::vector<octopus::Filter> > &filters, Rendexptr fill) {
    if (filters.has_value()) {
        for (const octopus::Filter &filter : filters.value()) {
            if (filter.visible)
                fill = applyFilter(fill, filter);
        }
    }
    return fill;
}

RendexSubtree assembleShapeLayer(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags) {
    struct StrokeRendexptr {
        Rendexptr render;
        octopus::Stroke::Position position;
        octopus::BlendMode blendMode;
    };

    ODE_ASSERT(layer->shape.has_value());

    Facets facets;
    requestFacets(layer, facets, maskBasis);

    Rendexptr body = drawLayerBody(layer);
    Rendexptr bodyAndStrokes = body;

    std::vector<std::pair<Rendexptr, octopus::BlendMode> > fillRenders;
    std::vector<StrokeRendexptr> strokeRenders;
    fillRenders.reserve(layer->shape->fills.size());
    strokeRenders.reserve(layer->shape->strokes.size());

    Rendexptr combinedFill;
    for (size_t i = 0; i < layer->shape->fills.size(); ++i) {
        const octopus::Fill &fill = layer->shape->fills[i];
        if (fill.visible) {
            Rendexptr fillRender = applyFilters(fill.filters, drawLayerFill(layer, i));
            fillRenders.push_back(std::make_pair(fillRender, fill.blendMode));
            combinedFill = blend(combinedFill, fillRender, fill.blendMode);
        }
    }

    for (size_t i = 0; i < layer->shape->strokes.size(); ++i) {
        const octopus::Shape::Stroke &stroke = layer->shape->strokes[i];
        if (stroke.visible) {
            Rendexptr strokeBody = drawLayerStroke(layer, i);
            if (stroke.position != octopus::Stroke::Position::INSIDE)
                bodyAndStrokes = blend(bodyAndStrokes, strokeBody, octopus::BlendMode::NORMAL);
            if (stroke.fill.visible) {
                Rendexptr strokeFill = applyFilters(stroke.fill.filters, drawLayerStrokeFill(layer, i));
                strokeRenders.push_back(StrokeRendexptr {
                    mask(strokeFill, strokeBody, ChannelMatrix { { 0, 0, 0, 1, 0 } }),
                    stroke.position,
                    stroke.fill.blendMode
                });
            }
        }
    }

    facets.set(Facets::BODY, body);
    facets.set(Facets::BODY_AND_STROKES, bodyAndStrokes);
    facets.set(Facets::FILL, mask(combinedFill, body, ChannelMatrix { { 0, 0, 0, 1, 0 } }));

    Rendexptr underlay = layerBackground(layer);
    assembleBackgroundLayerEffects(layer, bounds, facets, underlay);
    Rendexptr overlay = underlay;
    if (layer->blendMode == octopus::BlendMode::PASS_THROUGH) {
        for (const std::pair<Rendexptr, octopus::BlendMode> &fillRender : fillRenders) {
            overlay = blend(overlay, fillRender.first, fillRender.second);
        }
    } else
        overlay = blend(overlay, applyFillReplacementEffects(layer, bounds, facets, combinedFill), layer->blendMode);
    Rendexptr *inlayPoint = nullptr;
    assembleForegroundLayerEffects(layer, bounds, facets, underlay, overlay, inlayPoint);
    for (const StrokeRendexptr &stroke : strokeRenders) {
        if (stroke.position == octopus::Stroke::Position::CENTER && underlay == overlay) {
            // Optimization
            overlay = underlay = blend(underlay, stroke.render, stroke.blendMode);
        } else {
            if (stroke.position != octopus::Stroke::Position::INSIDE)
                underlay = blend(underlay, stroke.render, stroke.blendMode);
            if (stroke.position != octopus::Stroke::Position::OUTSIDE)
                overlay = blend(overlay, stroke.render, stroke.blendMode);
        }
    }

    return finalizeLayerAssembly(layer, bounds, facets, underlay, overlay, body, inlayPoint, maskBasis, flags);
}

RendexSubtree assembleTextLayer(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags) {
    ODE_ASSERT(layer && layer->type == octopus::Layer::Type::TEXT);

    Facets facets;
    requestFacets(layer, facets, maskBasis);

    Rendexptr textRender = drawLayerText(layer);
    facets.set(Facets::BODY, textRender);
    facets.set(Facets::BODY_AND_STROKES, textRender);
    facets.set(Facets::FILL, textRender);

    Rendexptr underlay = layerBackground(layer);
    assembleBackgroundLayerEffects(layer, bounds, facets, underlay);

    Rendexptr layerRender = applyFillReplacementEffects(layer, bounds, facets, textRender);
    if (layerRender != textRender)
        facets.set(Facets::FILL, mask(layerRender, textRender, ChannelMatrix { { 0, 0, 0, 1, 0 } }));
    Rendexptr overlay = blendIgnoreAlpha(underlay, layerRender, layer->blendMode);

    Rendexptr *inlayPoint = nullptr;
    assembleForegroundLayerEffects(layer, bounds, facets, underlay, overlay, inlayPoint);
    return finalizeLayerAssembly(layer, bounds, facets, underlay, overlay, textRender, inlayPoint, maskBasis, flags);
}

GroupLayerAssembler::GroupLayerAssembler(const LayerInstanceSpecifier &layer, const LayerBounds &bounds, const nonstd::optional<octopus::MaskBasis> &maskBasis, int flags) :
    layer(layer), bounds(bounds), groupMaskBasis(maskBasis), maskBasis(layer->maskBasis), flags(flags)
{
    content = layerBackground(layer);
}

void GroupLayerAssembler::setMask(const LayerInstanceSpecifier &layer, const RendexSubtree &subtree) {
    if (maskBasis.value_or(octopus::MaskBasis::BODY) == octopus::MaskBasis::SOLID) {
        if (layer->visible)
            addLayer(subtree);
        return;
    }
    maskSubtree = subtree;
    if ((maskVisible = layer->visible) && !embedInMask(maskBasis)) {
        // ISSUE: mask's alpha will be applied twice on mask layer
        content = setBackground(subtree.root, content);
    }
}

void GroupLayerAssembler::addLayer(const RendexSubtree &subtree) {
    content = setBackground(subtree.root, content);
}

RendexSubtree GroupLayerAssembler::get() {
    if (embedInMask(maskBasis) && maskVisible) {
        ODE_ASSERT(maskSubtree.inlayPoint);
        *maskSubtree.inlayPoint = setBackground(content, *maskSubtree.inlayPoint);
        content = maskSubtree.root;
    }

    Facets facets;
    requestFacets(layer, facets, groupMaskBasis);

    if (layer->blendMode == octopus::BlendMode::PASS_THROUGH) {
        RendexSubtree subtree;
        if (facets.requested(Facets::BODY) || facets.requested(Facets::BODY_AND_STROKES) || facets.requested(Facets::FILL)) {
            // TODO LAYER_AND_EFFECTS facet
            Rendexptr standalone = unsetBackground(content);
            facets.set(Facets::BODY, standalone);
            facets.set(Facets::BODY_AND_STROKES, standalone);
            facets.set(Facets::FILL, standalone);

            // TODO effects are not properly blended!
            // Instead, each overlay effect should be masked separately and directly into the resulting render
            Rendexptr underlay, overlay;
            assembleBackgroundLayerEffects(layer, bounds, facets, underlay);
            Rendexptr *inlayPoint = nullptr;
            assembleForegroundLayerEffects(layer, bounds, facets, underlay, overlay, inlayPoint);
            if (embedInMask(groupMaskBasis))
                overlay = makeInlayPoint(overlay, inlayPoint);
            overlay = mixMask(underlay, overlay, standalone, ChannelMatrix { { 0, 0, 0, 1, 0 } });
            subtree.root = blend(content, overlay, octopus::BlendMode::NORMAL);
            subtree.inlayPoint = inlayPoint;
            if (isMaskActive(groupMaskBasis))
                subtree.maskFacet = facets.get(groupMaskBasis.value());
        } else {
            subtree.root = content;
        }
        if (flags&ASSEMBLY_FLAG_FIXED_OPACITY)
            subtree.root = mix(makeBackground(), subtree.root, layer->opacity);
        else
            subtree.root = mixLayerOpacity(layer, makeBackground(), subtree.root);
        if (isMaskActive(maskBasis) && !embedInMask(maskBasis))
            subtree.root = mixMask(makeBackground(), subtree.root, maskSubtree.maskFacet, getChannelMatrix(layer->maskChannels));
        return subtree;
    } else {
        content = unsetBackground(content);
        facets.set(Facets::BODY, content);
        facets.set(Facets::BODY_AND_STROKES, content);
        facets.set(Facets::FILL, content);

        Rendexptr underlay = layerBackground(layer);
        assembleBackgroundLayerEffects(layer, bounds, facets, underlay);
        Rendexptr layerRender = applyFillReplacementEffects(layer, bounds, facets, content);
        if (layerRender != content)
            facets.set(Facets::FILL, mask(layerRender, content, ChannelMatrix { { 0, 0, 0, 1, 0 } }));
        Rendexptr overlay = blendIgnoreAlpha(underlay, layerRender, layer->blendMode);

        Rendexptr *inlayPoint = nullptr;
        assembleForegroundLayerEffects(layer, bounds, facets, underlay, overlay, inlayPoint);
        RendexSubtree subtree = finalizeLayerAssembly(layer, bounds, facets, underlay, overlay, content, inlayPoint, groupMaskBasis, flags);
        if (isMaskActive(maskBasis) && !embedInMask(maskBasis))
            subtree.root = mixMask(makeBackground(), subtree.root, maskSubtree.maskFacet, getChannelMatrix(layer->maskChannels));
        return subtree;
    }
}

}
