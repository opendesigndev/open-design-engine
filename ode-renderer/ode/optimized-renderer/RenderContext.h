
#pragma once

#include <stack>
#include <ode-logic.h>
#include "../image/ImageBase.h"
#include "Renderer.h"

namespace ode {

class RenderContext {

public:
    inline RenderContext(Renderer &renderer, ImageBase &imageBase, Component &component, double scale, const PixelBounds &bounds, double time) : renderer(renderer), imageBase(imageBase), component(component), scale(scale), bounds(bounds), time(time) { }
    const Rendexpr *step(const Rendexpr *expr, int entry);
    PlacedImagePtr peek() const;
    PlacedImagePtr finish();

private:
    class CacheKey : public std::pair<const Rendexpr *, std::stack<const Rendexpr *> > {
    public:
        CacheKey(RenderContext *ctx, const Rendexpr *expr);
    };

    Renderer &renderer;
    ImageBase &imageBase;
    Component &component;
    double scale;
    PixelBounds bounds;
    double time;
    std::stack<PlacedImagePtr> imageStack;
    std::stack<const Rendexpr *> backgroundStack;
    std::stack<const Rendexpr *> backgroundAntiStack;
    std::map<CacheKey, std::pair<PlacedImagePtr, int> > imageCache;

    const Rendexpr *stepUncached(const Rendexpr *expr, int entry);

};

}
