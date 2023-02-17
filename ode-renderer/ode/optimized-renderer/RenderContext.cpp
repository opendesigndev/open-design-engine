
#include "RenderContext.h"

#include <ode/animation/animate.h>

namespace ode {

static const EmptyExpression EMPTY_EXPRESSION;

RenderContext::CacheKey::CacheKey(RenderContext *ctx, const Rendexpr *expr) : std::pair<const Rendexpr *, std::stack<const Rendexpr *> >(
    expr->type == BackgroundExpression::TYPE ? nullptr : expr,
    expr->type == BackgroundExpression::TYPE ? ctx->backgroundStack : std::stack<const Rendexpr *>()
) { }

const Rendexpr *RenderContext::step(const Rendexpr *expr, int entry) {
    ODE_ASSERT(expr);

    if (!entry) {
        std::map<CacheKey, std::pair<PlacedImagePtr, int> >::iterator it = imageCache.find(CacheKey(this, expr));
        if (it != imageCache.end()) {
            imageStack.push(it->second.first);
            // TODO BACKGROUNDS ARE CURRENTLY NOT ERASED - WASTE OF VIDEO MEMORY !!!
            if (++it->second.second >= expr->refs && expr->type != BackgroundExpression::TYPE)
                imageCache.erase(it);
            return nullptr;
        }
    }

    if (const Rendexpr *result = stepUncached(expr, entry))
        return result;
    else if (expr->refs > 1 || expr->type == BackgroundExpression::TYPE) { // TODO BACKGROUNDS - see above
        ODE_ASSERT(!imageStack.empty());
        // Important: CacheKey object must be created AFTER stepUncached
        imageCache.insert(std::make_pair(CacheKey(this, expr), std::make_pair(imageStack.top(), 1)));
    }

    return nullptr;
}

const Rendexpr *RenderContext::stepUncached(const Rendexpr *expr, int entry) {
    #define NONNULL(x) ((x) ? (x) : &EMPTY_EXPRESSION)

    switch (expr->type) {

        case EmptyExpression::TYPE:
            imageStack.push(PlacedImagePtr());
            return nullptr;

        case IdentityExpression::TYPE:
            return entry == 0 ? NONNULL(static_cast<const IdentityExpression *>(expr)->content.get()) : nullptr;

        case BlendExpression::TYPE:
            {
                const BlendExpression *blendExpr = static_cast<const BlendExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(blendExpr->dst.get());
                    case 1:
                        return NONNULL(blendExpr->src.get());
                    case 2:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr src = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr dst = imageStack.top();
                            imageStack.top() = renderer.blend(dst, src, blendExpr->blendMode);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case BlendIgnoreAlphaExpression::TYPE:
            {
                const BlendIgnoreAlphaExpression *blendExpr = static_cast<const BlendIgnoreAlphaExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(blendExpr->dst.get());
                    case 1:
                        return NONNULL(blendExpr->src.get());
                    case 2:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr src = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr dst = imageStack.top();
                            imageStack.top() = renderer.blendIgnoreAlpha(dst, src, blendExpr->blendMode);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case MaskExpression::TYPE:
            {
                const MaskExpression *maskExpr = static_cast<const MaskExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(maskExpr->image.get());
                    case 1:
                        return NONNULL(maskExpr->mask.get());
                    case 2:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr mask = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr image = imageStack.top();
                            imageStack.top() = renderer.mask(image, mask, maskExpr->channelMatrix);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case MixMaskExpression::TYPE:
            {
                const MixMaskExpression *mixMaskExpr = static_cast<const MixMaskExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(mixMaskExpr->dst.get());
                    case 1:
                        return NONNULL(mixMaskExpr->src.get());
                    case 2:
                        return NONNULL(mixMaskExpr->mask.get());
                    case 3:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr mask = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr src = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr dst = imageStack.top();
                            imageStack.top() = renderer.mixMask(dst, src, mask, mixMaskExpr->channelMatrix);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case MixExpression::TYPE:
            {
                const MixExpression *mixExpr = static_cast<const MixExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(mixExpr->a.get());
                    case 1:
                        return NONNULL(mixExpr->b.get());
                    case 2:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr b = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr a = imageStack.top();
                            imageStack.top() = renderer.mix(a, b, mixExpr->ratio);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case MultiplyAlphaExpression::TYPE:
            {
                const MultiplyAlphaExpression *multiplyAlphaExpr = static_cast<const MultiplyAlphaExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(multiplyAlphaExpr->image.get());
                    case 1:
                        ODE_ASSERT(!imageStack.empty());
                        imageStack.top() = renderer.multiplyAlpha(imageStack.top(), multiplyAlphaExpr->multiplier);
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case DrawLayerBodyExpression::TYPE:
            imageStack.push(renderer.drawLayerBody(component, static_cast<const DrawLayerBodyExpression *>(expr)->layer, scale, time));
            return nullptr;

        case DrawLayerStrokeExpression::TYPE:
            {
                const DrawLayerStrokeExpression *drawExpr = static_cast<const DrawLayerStrokeExpression *>(expr);
                imageStack.push(renderer.drawLayerStroke(component, drawExpr->layer, drawExpr->index, scale, time));
                return nullptr;
            }

        case DrawLayerFillExpression::TYPE:
            {
                const DrawLayerFillExpression *drawExpr = static_cast<const DrawLayerFillExpression *>(expr);
                imageStack.push(renderer.drawLayerFill(component, drawExpr->layer, drawExpr->index, imageBase, scale, time));
                return nullptr;
            }

        case DrawLayerStrokeFillExpression::TYPE:
            {
                const DrawLayerStrokeFillExpression *drawExpr = static_cast<const DrawLayerStrokeFillExpression *>(expr);
                imageStack.push(renderer.drawLayerStrokeFill(component, drawExpr->layer, drawExpr->index, imageBase, scale, time));
                return nullptr;
            }

        case DrawLayerTextExpression::TYPE:
            imageStack.push(renderer.drawLayerText(component, static_cast<const DrawLayerTextExpression *>(expr)->layer, scale, time));
            return nullptr;

        case DrawLayerEffectExpression::TYPE:
            {
                const DrawLayerEffectExpression *drawExpr = static_cast<const DrawLayerEffectExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(drawExpr->basis.get());
                    case 1:
                        ODE_ASSERT(!imageStack.empty());
                        imageStack.top() = renderer.drawLayerEffect(component, drawExpr->layer, drawExpr->index, imageBase, imageStack.top(), scale, time);
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case ApplyFilterExpression::TYPE:
            {
                const ApplyFilterExpression *filterExpr = static_cast<const ApplyFilterExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(filterExpr->basis.get());
                    case 1:
                        ODE_ASSERT(!imageStack.empty());
                        imageStack.top() = renderer.applyFilter(filterExpr->filter, imageStack.top());
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case BackgroundExpression::TYPE:
            {
                const BackgroundExpression *bgExpr = static_cast<const BackgroundExpression *>(expr);
                switch (entry) {
                    case 0:
                        {
                            if (const Rendexpr *background = backgroundStack.empty() ? nullptr : backgroundStack.top()) {
                                backgroundStack.pop();
                                backgroundAntiStack.push(background);
                                return background;
                            } else {
                                imageStack.push(PlacedImagePtr());
                                return nullptr;
                            }
                        }
                    case 1:
                        ODE_ASSERT(!backgroundAntiStack.empty());
                        backgroundStack.push(backgroundAntiStack.top());
                        backgroundAntiStack.pop();
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case SetBackgroundExpression::TYPE:
            {
                const SetBackgroundExpression *setBgExpr = static_cast<const SetBackgroundExpression *>(expr);
                switch (entry) {
                    case 0:
                        backgroundStack.push(setBgExpr->background.get());
                        return NONNULL(setBgExpr->content.get());
                    case 1:
                        ODE_ASSERT(!backgroundStack.empty());
                        backgroundStack.pop();
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case MixLayerOpacityExpression::TYPE:
            {
                const MixLayerOpacityExpression *mixExpr = static_cast<const MixLayerOpacityExpression *>(expr);
                // Optimization: Check if layerOpacity is 0 or 1 - only traverse one branch if so
                double layerOpacity = mixExpr->layer->opacity;
                if (Result<const DocumentAnimation *, DesignError> anim = component.getAnimation(mixExpr->layer->id)) {
                    ODE_ASSERT(anim.value());
                    for (const LayerAnimation &animation : anim.value()->animations) {
                        if (animation.type == LayerAnimation::OPACITY)
                            layerOpacity *= animateOpacity(animation, time);
                    }
                }
                switch (entry) {
                    case 0:
                        return NONNULL(layerOpacity != 1 ? mixExpr->a.get() : nullptr);
                    case 1:
                        return NONNULL(layerOpacity != 0 ? mixExpr->b.get() : nullptr);
                    case 2:
                        {
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr b = imageStack.top();
                            imageStack.pop();
                            ODE_ASSERT(!imageStack.empty());
                            PlacedImagePtr a = imageStack.top();
                            ODE_ASSERT(mixExpr->layer);
                            if (layerOpacity == 0)
                                imageStack.top() = a;
                            else if (layerOpacity == 1)
                                imageStack.top() = b;
                            else
                                imageStack.top() = renderer.mix(a, b, layerOpacity);
                            return nullptr;
                        }
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }
    }

    ODE_ASSERT(!"Invalid render expression type");
    return nullptr;
}

PlacedImagePtr RenderContext::peek() const {
    ODE_ASSERT(!imageStack.empty());
    return imageStack.top();
}

PlacedImagePtr RenderContext::finish() {
    ODE_ASSERT(imageStack.size() == 1);
    if (!imageStack.empty()) {
        if (component.getOctopus().dimensions.has_value())
            return renderer.reframe(imageStack.top(), outerPixelBounds(scaleBounds(UnscaledBounds(0, 0, component.getOctopus().dimensions->width, component.getOctopus().dimensions->height), scale)));
        else
            return imageStack.top();
    }
    return nullptr;
}

}
