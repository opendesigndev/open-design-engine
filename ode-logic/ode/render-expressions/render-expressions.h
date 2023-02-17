
#pragma once

#include <octopus/octopus.h>
#include "../core/ChannelMatrix.h"
#include "../core/LayerInstanceSpecifier.h"
#include "RenderExpression.h"

namespace ode {

struct LayerRenderExpression : RenderExpression {
    LayerInstanceSpecifier layer;

    inline LayerRenderExpression(Type type, int flags, const LayerInstanceSpecifier &layer) : RenderExpression(type, flags), layer(layer) { }

    inline const octopus::Layer *getLayer() const override {
        return layer.layer;
    }
};

/// Results in an empty dimensionless image
struct EmptyExpression : RenderExpression {
    static constexpr Type TYPE = 0;

    inline EmptyExpression() : RenderExpression(TYPE, 0) { }
};

/// Simply relays its child expression without change (mainly used when a persisten pointer to a node is needed before its type is known)
struct IdentityExpression : RenderExpression {
    static constexpr Type TYPE = 1;
    Rendexptr content;

    inline IdentityExpression(int flags, const Rendexptr &content) : RenderExpression(TYPE, flags), content(content) { }
};

/// Blends src into dst using one of the Octopus blend modes and outputs the result
struct BlendExpression : RenderExpression {
    static constexpr Type TYPE = 2;
    Rendexptr dst, src;
    octopus::BlendMode blendMode;

    inline BlendExpression(int flags, const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode) : RenderExpression(TYPE, flags), dst(dst), src(src), blendMode(blendMode) { }
};

/// Blends src into dst but ignores src's alpha channel (as if it was fully opaque) - this is used when the alpha is applied separately in a different part of the graph
struct BlendIgnoreAlphaExpression : RenderExpression {
    static constexpr Type TYPE = 3;
    Rendexptr dst, src;
    octopus::BlendMode blendMode;

    inline BlendIgnoreAlphaExpression(int flags, const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode) : RenderExpression(TYPE, flags), dst(dst), src(src), blendMode(blendMode) { }
};

/// Multiplies the alpha channel of image by the product of channelMatrix and the channels of mask
struct MaskExpression : RenderExpression {
    static constexpr Type TYPE = 4;
    Rendexptr image, mask;
    ChannelMatrix channelMatrix;

    inline MaskExpression(int flags, const Rendexptr &image, const Rendexptr &mask, const ChannelMatrix &channelMatrix) : RenderExpression(TYPE, flags), image(image), mask(mask), channelMatrix(channelMatrix) { }
};

/// Produces the weighted average of dst and src, where the weight is the product of channelMatrix and the channels of mask
struct MixMaskExpression : RenderExpression {
    static constexpr Type TYPE = 5;
    Rendexptr dst, src, mask;
    ChannelMatrix channelMatrix;

    inline MixMaskExpression(int flags, const Rendexptr &dst, const Rendexptr &src, const Rendexptr &mask, const ChannelMatrix &channelMatrix) : RenderExpression(TYPE, flags), dst(dst), src(src), mask(mask), channelMatrix(channelMatrix) { }
};

/// Produces the weighted average of a and b with ratio as the weight
struct MixExpression : RenderExpression {
    static constexpr Type TYPE = 6;
    Rendexptr a, b;
    double ratio;

    inline MixExpression(int flags, const Rendexptr &a, const Rendexptr &b, double ratio) : RenderExpression(TYPE, flags), a(a), b(b), ratio(ratio) { }
};

/// Multiplies the alpha channel of image by the fixed multiplier
struct MultiplyAlphaExpression : RenderExpression {
    static constexpr Type TYPE = 7;
    Rendexptr image;
    double multiplier;

    inline MultiplyAlphaExpression(int flags, const Rendexptr &image, double multiplier) : RenderExpression(TYPE, flags), image(image), multiplier(multiplier) { }
};

/// Produces the image of a layer's body
struct DrawLayerBodyExpression : LayerRenderExpression {
    static constexpr Type TYPE = 8;

    inline DrawLayerBodyExpression(int flags, const LayerInstanceSpecifier &layer) : LayerRenderExpression(TYPE, flags, layer) { }
};

/// Produces the image of a layer's index-th stroke
struct DrawLayerStrokeExpression : LayerRenderExpression {
    static constexpr Type TYPE = 9;
    int index;

    inline DrawLayerStrokeExpression(int flags, const LayerInstanceSpecifier &layer, int index) : LayerRenderExpression(TYPE, flags, layer), index(index) { }
};

/// Produces the image of a layer's index-th fill
struct DrawLayerFillExpression : LayerRenderExpression {
    static constexpr Type TYPE = 10;
    int index;

    inline DrawLayerFillExpression(int flags, const LayerInstanceSpecifier &layer, int index) : LayerRenderExpression(TYPE, flags, layer), index(index) { }
};

/// Produces the image of a layer's index-th stroke's fill
struct DrawLayerStrokeFillExpression : LayerRenderExpression {
    static constexpr Type TYPE = 11;
    int index;

    inline DrawLayerStrokeFillExpression(int flags, const LayerInstanceSpecifier &layer, int index) : LayerRenderExpression(TYPE, flags, layer), index(index) { }
};

/// Produces the image of a text layer
struct DrawLayerTextExpression : LayerRenderExpression {
    static constexpr Type TYPE = 12;

    inline DrawLayerTextExpression(int flags, const LayerInstanceSpecifier &layer) : LayerRenderExpression(TYPE, flags, layer) { }
};

/// Produces the image of a layer's index-th effect applied on basis
struct DrawLayerEffectExpression : LayerRenderExpression {
    static constexpr Type TYPE = 13;
    Rendexptr basis;
    int index;

    inline DrawLayerEffectExpression(int flags, const Rendexptr &basis, const LayerInstanceSpecifier &layer, int index) : LayerRenderExpression(TYPE, flags, layer), basis(basis), index(index) { }
};

/// Applies the Octopus filter on basis and outputs the result
struct ApplyFilterExpression : RenderExpression {
    static constexpr Type TYPE = 14;
    Rendexptr basis;
    octopus::Filter filter;

    inline ApplyFilterExpression(int flags, const Rendexptr &basis, const octopus::Filter &filter) : RenderExpression(TYPE, flags), basis(basis), filter(filter) { }
};

/// Represents the background subtree of the closest ancestor SetBackground node where this node is in its content subtree
struct BackgroundExpression : RenderExpression {
    static constexpr Type TYPE = 15;

    inline BackgroundExpression() : RenderExpression(TYPE, 0) { }
};

/// Sets the background subtree for the background nodes within its content branch
struct SetBackgroundExpression : RenderExpression {
    static constexpr Type TYPE = 16;
    Rendexptr content;
    Rendexptr background;

    inline SetBackgroundExpression(int flags, const Rendexptr &content, const Rendexptr &background) : RenderExpression(TYPE, flags), content(content), background(background) { }
};

/// Produces the weighted average of a and b with layer's opacity as the weight
struct MixLayerOpacityExpression : LayerRenderExpression {
    static constexpr Type TYPE = 17;
    Rendexptr a, b;

    inline MixLayerOpacityExpression(int flags, const LayerInstanceSpecifier &layer, const Rendexptr &a, const Rendexptr &b) : LayerRenderExpression(TYPE, flags, layer), a(a), b(b) { }
};

constexpr const char *renderExpressionTypeShortName(int type) {
    switch (type) {
        case EmptyExpression::TYPE: return "Empty";
        case IdentityExpression::TYPE: return "Identity";
        case BlendExpression::TYPE: return "Blend";
        case BlendIgnoreAlphaExpression::TYPE: return "Blend-IA";
        case MaskExpression::TYPE: return "Mask";
        case MixMaskExpression::TYPE: return "MixMask";
        case MixExpression::TYPE: return "Mix";
        case MultiplyAlphaExpression::TYPE: return "AlphaMult";
        case DrawLayerBodyExpression::TYPE: return "LayerBody";
        case DrawLayerStrokeExpression::TYPE: return "LayerStroke";
        case DrawLayerFillExpression::TYPE: return "LayerFill";
        case DrawLayerStrokeFillExpression::TYPE: return "LayerStrokeFill";
        case DrawLayerTextExpression::TYPE: return "LayerText";
        case DrawLayerEffectExpression::TYPE: return "LayerEffect";
        case ApplyFilterExpression::TYPE: return "ApplyFilter";
        case BackgroundExpression::TYPE: return "BG";
        case SetBackgroundExpression::TYPE: return "SetBG";
        case MixLayerOpacityExpression::TYPE: return "MixLayerOpacity";
        default:
            return "???";
    }
}

}

#define RENDER_EXPRESSION_CASES(ACTION, OPERAND_ACTION) \
    case ::ode::EmptyExpression::TYPE: \
        ACTION(EmptyExpression); \
        break; \
    case ::ode::IdentityExpression::TYPE: \
        ACTION(IdentityExpression); \
        OPERAND_ACTION(IdentityExpression, content); \
        break; \
    case ::ode::BlendExpression::TYPE: \
        ACTION(BlendExpression); \
        OPERAND_ACTION(BlendExpression, dst); \
        OPERAND_ACTION(BlendExpression, src); \
        break; \
    case ::ode::BlendIgnoreAlphaExpression::TYPE: \
        ACTION(BlendIgnoreAlphaExpression); \
        OPERAND_ACTION(BlendIgnoreAlphaExpression, dst); \
        OPERAND_ACTION(BlendIgnoreAlphaExpression, src); \
        break; \
    case ::ode::MaskExpression::TYPE: \
        ACTION(MaskExpression); \
        OPERAND_ACTION(MaskExpression, image); \
        OPERAND_ACTION(MaskExpression, mask); \
        break; \
    case ::ode::MixMaskExpression::TYPE: \
        ACTION(MixMaskExpression); \
        OPERAND_ACTION(MixMaskExpression, dst); \
        OPERAND_ACTION(MixMaskExpression, src); \
        OPERAND_ACTION(MixMaskExpression, mask); \
        break; \
    case ::ode::MixExpression::TYPE: \
        ACTION(MixExpression); \
        OPERAND_ACTION(MixExpression, a); \
        OPERAND_ACTION(MixExpression, b); \
        break; \
    case ::ode::MultiplyAlphaExpression::TYPE: \
        ACTION(MultiplyAlphaExpression); \
        OPERAND_ACTION(MultiplyAlphaExpression, image); \
        break; \
    case ::ode::DrawLayerBodyExpression::TYPE: \
        ACTION(DrawLayerBodyExpression); \
        break; \
    case ::ode::DrawLayerStrokeExpression::TYPE: \
        ACTION(DrawLayerStrokeExpression); \
        break; \
    case ::ode::DrawLayerFillExpression::TYPE: \
        ACTION(DrawLayerFillExpression); \
        break; \
    case ::ode::DrawLayerStrokeFillExpression::TYPE: \
        ACTION(DrawLayerStrokeFillExpression); \
        break; \
    case ::ode::DrawLayerTextExpression::TYPE: \
        ACTION(DrawLayerTextExpression); \
        break; \
    case ::ode::DrawLayerEffectExpression::TYPE: \
        ACTION(DrawLayerEffectExpression); \
        OPERAND_ACTION(DrawLayerEffectExpression, basis); \
        break; \
    case ::ode::ApplyFilterExpression::TYPE: \
        ACTION(ApplyFilterExpression); \
        OPERAND_ACTION(ApplyFilterExpression, basis); \
        break; \
    case ::ode::BackgroundExpression::TYPE: \
        ACTION(BackgroundExpression); \
        break; \
    case ::ode::SetBackgroundExpression::TYPE: \
        ACTION(SetBackgroundExpression); \
        OPERAND_ACTION(SetBackgroundExpression, content); \
        OPERAND_ACTION(SetBackgroundExpression, background); \
        break; \
    case ::ode::MixLayerOpacityExpression::TYPE: \
        ACTION(MixLayerOpacityExpression); \
        OPERAND_ACTION(MixLayerOpacityExpression, a); \
        OPERAND_ACTION(MixLayerOpacityExpression, b); \
        break; \

