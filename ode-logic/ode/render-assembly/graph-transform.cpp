
#include "graph-transform.h"

#include <stack>
#include <map>
#include "../render-expressions/render-operations.h"

namespace ode {

static const EmptyExpression EMPTY_EXPRESSION;

class GraphTransformContext {

public:
    const Rendexpr *step(const Rendexpr *expr, int entry);
    Rendexptr peek() const;
    Rendexptr finish();

private:
    class CacheKey : public std::pair<const Rendexpr *, std::stack<const Rendexpr *> > {
    public:
        CacheKey(GraphTransformContext *ctx, const Rendexpr *expr);
    };

    std::stack<Rendexptr> outputStack;
    std::stack<const Rendexpr *> backgroundStack;
    std::stack<const Rendexpr *> backgroundAntiStack;
    std::map<CacheKey, std::pair<Rendexptr, int> > outputCache;

    const Rendexpr *stepUncached(const Rendexpr *expr, int entry);

};

GraphTransformContext::CacheKey::CacheKey(GraphTransformContext *ctx, const Rendexpr *expr) : std::pair<const Rendexpr *, std::stack<const Rendexpr *> >(
    expr->type == BackgroundExpression::TYPE ? nullptr : expr,
    expr->type == BackgroundExpression::TYPE ? ctx->backgroundStack : std::stack<const Rendexpr *>()
) { }

const Rendexpr *GraphTransformContext::step(const Rendexpr *expr, int entry) {
    ODE_ASSERT(expr);

    if (!entry) {
        std::map<CacheKey, std::pair<Rendexptr, int> >::iterator it = outputCache.find(CacheKey(this, expr));
        if (it != outputCache.end()) {
            outputStack.push(it->second.first);
            // Erasure of background nodes is not solved (they stay in cache indefinitely)
            if (++it->second.second >= expr->refs && expr->type != BackgroundExpression::TYPE)
                outputCache.erase(it);
            return nullptr;
        }
    }

    if (const Rendexpr *result = stepUncached(expr, entry))
        return result;
    else if (expr->refs > 1 || expr->type == BackgroundExpression::TYPE) { // TODO BACKGROUNDS - see above
        ODE_ASSERT(!outputStack.empty());
        // Important: CacheKey object must be created AFTER stepUncached
        outputCache.insert(std::make_pair(CacheKey(this, expr), std::make_pair(outputStack.top(), 1)));
    }

    return nullptr;
}

const Rendexpr *GraphTransformContext::stepUncached(const Rendexpr *expr, int entry) {
    #define NONNULL(x) ((x) ? (x) : &EMPTY_EXPRESSION)

    switch (expr->type) {

        case EmptyExpression::TYPE:
            outputStack.push(Rendexptr());
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
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr src = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr dst = outputStack.top();
                            outputStack.top() = blend(dst, src, blendExpr->blendMode);
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
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr src = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr dst = outputStack.top();
                            outputStack.top() = blendIgnoreAlpha(dst, src, blendExpr->blendMode);
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
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr m = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr image = outputStack.top();
                            outputStack.top() = mask(image, m, maskExpr->channelMatrix);
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
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr mask = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr src = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr dst = outputStack.top();
                            outputStack.top() = mixMask(dst, src, mask, mixMaskExpr->channelMatrix);
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
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr b = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr a = outputStack.top();
                            outputStack.top() = mix(a, b, mixExpr->ratio);
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
                        ODE_ASSERT(!outputStack.empty());
                        outputStack.top() = multiplyAlpha(outputStack.top(), multiplyAlphaExpr->multiplier);
                        return nullptr;
                }
                ODE_ASSERT(!"Invalid entry");
                return nullptr;
            }

        case DrawLayerBodyExpression::TYPE:
            outputStack.push(drawLayerBody(static_cast<const DrawLayerBodyExpression *>(expr)->layer));
            return nullptr;

        case DrawLayerStrokeExpression::TYPE:
            {
                const DrawLayerStrokeExpression *drawExpr = static_cast<const DrawLayerStrokeExpression *>(expr);
                outputStack.push(drawLayerStroke(drawExpr->layer, drawExpr->index));
                return nullptr;
            }

        case DrawLayerFillExpression::TYPE:
            {
                const DrawLayerFillExpression *drawExpr = static_cast<const DrawLayerFillExpression *>(expr);
                outputStack.push(drawLayerFill(drawExpr->layer, drawExpr->index));
                return nullptr;
            }

        case DrawLayerStrokeFillExpression::TYPE:
            {
                const DrawLayerStrokeFillExpression *drawExpr = static_cast<const DrawLayerStrokeFillExpression *>(expr);
                outputStack.push(drawLayerStrokeFill(drawExpr->layer, drawExpr->index));
                return nullptr;
            }

        case DrawLayerTextExpression::TYPE:
            outputStack.push(drawLayerText(static_cast<const DrawLayerTextExpression *>(expr)->layer));
            return nullptr;

        case DrawLayerEffectExpression::TYPE:
            {
                const DrawLayerEffectExpression *drawExpr = static_cast<const DrawLayerEffectExpression *>(expr);
                switch (entry) {
                    case 0:
                        return NONNULL(drawExpr->basis.get());
                    case 1:
                        ODE_ASSERT(!outputStack.empty());
                        outputStack.top() = drawLayerEffect(drawExpr->basis, drawExpr->layer, drawExpr->index);
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
                        ODE_ASSERT(!outputStack.empty());
                        outputStack.top() = applyFilter(filterExpr->basis, filterExpr->filter);
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
                                outputStack.push(Rendexptr());
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
                switch (entry) {
                    case 0:
                        return NONNULL(mixExpr->a.get());
                    case 1:
                        return NONNULL(mixExpr->b.get());
                    case 2:
                        {
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr b = outputStack.top();
                            outputStack.pop();
                            ODE_ASSERT(!outputStack.empty());
                            Rendexptr a = outputStack.top();
                            ODE_ASSERT(mixExpr->layer);
                            outputStack.top() = mixLayerOpacity(mixExpr->layer, a, b);
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

Rendexptr GraphTransformContext::peek() const {
    ODE_ASSERT(!outputStack.empty());
    return outputStack.top();
}

Rendexptr GraphTransformContext::finish() {
    ODE_ASSERT(outputStack.size() == 1);
    if (!outputStack.empty())
        return outputStack.top();
    return nullptr;
}

Rendexptr resolveBackground(const Rendexptr &root) {
    if (!root)
        return nullptr;

    GraphTransformContext transformContext;
    std::stack<std::pair<const Rendexpr *, int> > exprStack;

    exprStack.push(std::make_pair(root.get(), 0));
    while (!exprStack.empty()) {
        std::pair<const Rendexpr *, int> &top = exprStack.top();
        if (const Rendexpr *child = transformContext.step(top.first, top.second++))
            exprStack.push(std::make_pair(child, 0));
        else
            exprStack.pop();
    }
    return transformContext.finish();
}

}
