
#include "render-operations.h"

#include "render-expressions.h"

namespace ode {

Rendexptr blend(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode) {
    // Not actual blending mode
    ODE_ASSERT(blendMode != octopus::BlendMode::PASS_THROUGH);
    if (!src)
        return dst;
    if (!dst)
        return src;
    ++dst->refs, ++src->refs;
    return Rendexptr(new BlendExpression(dst->flags|src->flags, dst, src, blendMode));
}

Rendexptr blend(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode, Rendexptr *&inlayPoint) {
    // Not actual blending modes
    ODE_ASSERT(blendMode != octopus::BlendMode::PASS_THROUGH);
    if (!src)
        return dst;
    if (!dst)
        return makeInlayPoint(src, inlayPoint);
    ++dst->refs, ++src->refs;
    BlendExpression *result = new BlendExpression(dst->flags|src->flags, dst, src, blendMode);
    if (!inlayPoint)
        inlayPoint = &result->dst;
    return Rendexptr(result);
}

Rendexptr blendIgnoreAlpha(const Rendexptr &dst, const Rendexptr &src, octopus::BlendMode blendMode) {
    // Not actual blending mode
    ODE_ASSERT(blendMode != octopus::BlendMode::PASS_THROUGH);
    if (!src)
        return dst;
    if (!dst)
        return src;
    ++dst->refs, ++src->refs;
    return Rendexptr(new BlendIgnoreAlphaExpression(dst->flags|src->flags, dst, src, blendMode));
}

Rendexptr mask(const Rendexptr &image, const Rendexptr &mask, const ChannelMatrix &channelMatrix) {
    if (image && mask && (channelMatrix.m[3] > 0 || channelMatrix.m[1] > 0 || channelMatrix.m[4] > 0 || channelMatrix.m[2] > 0 || channelMatrix.m[0] > 0)) {
        ++image->refs, ++mask->refs;
        return Rendexptr(new MaskExpression(image->flags|mask->flags, image, mask, channelMatrix));
    }
    return Rendexptr();
}

Rendexptr mixMask(const Rendexptr &dst, const Rendexptr &src, const Rendexptr &mask, const ChannelMatrix &channelMatrix) {
    if (!(src && mask && (channelMatrix.m[3] > 0 || channelMatrix.m[1] > 0 || channelMatrix.m[4] > 0 || channelMatrix.m[2] > 0 || channelMatrix.m[0] > 0)))
        return dst;
    if (!dst)
        return ode::mask(src, mask, channelMatrix);
    if (dst == src)
        return dst;

    // Common case to optimize - blend-IA src0 into dst0 and then mixMask that into dst0 with src0 mask - equivalent to blend src0 into dst0
    if (src->type == BlendIgnoreAlphaExpression::TYPE) {
        const BlendIgnoreAlphaExpression *blendSrc = static_cast<const BlendIgnoreAlphaExpression *>(src.get());
        if (blendSrc->src == mask && blendSrc->dst == dst)
            return Rendexptr(new BlendExpression(blendSrc->flags, dst, mask, blendSrc->blendMode));
    }

    ++dst->refs, ++src->refs, ++mask->refs;
    return Rendexptr(new MixMaskExpression(dst->flags|src->flags|mask->flags, dst, src, mask, channelMatrix));
}

Rendexptr mix(const Rendexptr &a, const Rendexptr &b, double ratio) {
    if (ratio == 0)
        return a;
    if (ratio == 1)
        return b;
    if (!a)
        return multiplyAlpha(b, ratio);
    if (!b)
        return multiplyAlpha(a, 1-ratio);
    ++a->refs, ++b->refs;
    return Rendexptr(new MixExpression(a->flags|b->flags, a, b, ratio));
}

Rendexptr multiplyAlpha(const Rendexptr &image, double multiplier) {
    if (multiplier == 1)
        return image;
    if (multiplier <= 0 || !image)
        return Rendexptr();
    ++image->refs;
    return Rendexptr(new MultiplyAlphaExpression(image->flags, image, multiplier));
}

Rendexptr drawLayerBody(const LayerInstanceSpecifier &layer) {
    if (!layer)
        return Rendexptr();
    return Rendexptr(new DrawLayerBodyExpression(0, layer));
}

Rendexptr drawLayerStroke(const LayerInstanceSpecifier &layer, int index) {
    if (!layer)
        return Rendexptr();
    ODE_ASSERT(layer->shape.has_value() && index >= 0 && index < int(layer->shape->strokes.size()));
    return Rendexptr(new DrawLayerStrokeExpression(0, layer, index));
}

Rendexptr drawLayerFill(const LayerInstanceSpecifier &layer, int index) {
    if (!layer)
        return Rendexptr();
    ODE_ASSERT(layer->shape.has_value() && index >= 0 && index < int(layer->shape->fills.size()));
    return Rendexptr(new DrawLayerFillExpression(0, layer, index));
}

Rendexptr drawLayerStrokeFill(const LayerInstanceSpecifier &layer, int index) {
    if (!layer)
        return Rendexptr();
    ODE_ASSERT(layer->shape.has_value() && index >= 0 && index < int(layer->shape->strokes.size()));
    return Rendexptr(new DrawLayerStrokeFillExpression(0, layer, index));
}

Rendexptr drawLayerText(const LayerInstanceSpecifier &layer) {
    if (!layer)
        return Rendexptr();
    ODE_ASSERT(layer->text.has_value());
    return Rendexptr(new DrawLayerTextExpression(0, layer));
}

Rendexptr drawLayerEffect(const Rendexptr &basis, const LayerInstanceSpecifier &layer, int index) {
    if (!layer)
        return Rendexptr();
    ODE_ASSERT(index >= 0 && index < int(layer->effects.size()));
    if (!basis && layer->effects[index].type != octopus::Effect::Type::OVERLAY) // basis is required for all except OVERLAY effects
        return Rendexptr();
    if (basis)
        ++basis->refs;
    return Rendexptr(new DrawLayerEffectExpression(basis ? basis->flags : 0, basis, layer, index));
}

Rendexptr applyFilter(const Rendexptr &basis, const octopus::Filter &filter) {
    if (!basis)
        return Rendexptr();
    if (filter.type == octopus::Filter::Type::OPACITY_MULTIPLIER)
        return multiplyAlpha(basis, filter.opacity.value_or(1));
    // TODO maybe convert to ColorAdjusmentExpression?
    ++basis->refs;
    return Rendexptr(new ApplyFilterExpression(basis->flags, basis, filter));
}

Rendexptr makeInlayPoint(Rendexptr content, Rendexptr *&inlayPoint) {
    if (!inlayPoint) {
        if (!content)
            content = Rendexptr(new EmptyExpression);
        ++content->refs;
        IdentityExpression *result = new IdentityExpression(content->flags, content);
        inlayPoint = &result->content;
        return Rendexptr(result);
    }
    return content;
}

Rendexptr makeBackground() {
    return Rendexptr(new BackgroundExpression);
}

Rendexptr setBackground(const Rendexptr &content, const Rendexptr &background) {
    if (!background)
        return unsetBackground(content);
    if (!content)
        return Rendexptr();
    if (background->type == BackgroundExpression::TYPE)
        return content;
    if (content->type == BackgroundExpression::TYPE)
        return background;
    ++content->refs;
    if (background)
        ++background->refs;
    return Rendexptr(new SetBackgroundExpression(content->flags, content, background));
}

Rendexptr unsetBackground(const Rendexptr &content) {
    if (!content)
        return Rendexptr();
    ++content->refs;
    return Rendexptr(new SetBackgroundExpression(content->flags, content, nullptr));
}

Rendexptr mixLayerOpacity(const LayerInstanceSpecifier &layer, const Rendexptr &a, const Rendexptr &b) {
    return Rendexptr(new MixLayerOpacityExpression(0, layer, a, b));
}

}
