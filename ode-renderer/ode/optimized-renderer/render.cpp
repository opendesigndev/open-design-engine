
#include "render.h"

#include "Renderer.h"
#include "RenderContext.h"

namespace ode {

PlacedImagePtr render(Renderer &renderer, ImageBase &imageBase, Component &component, const Rendexptr &root, double scale, const PixelBounds &bounds, double time) {
    if (!root)
        return renderer.reframe(nullptr, bounds);

    RenderContext renderContext(renderer, imageBase, component, scale, bounds, time);
    std::stack<std::pair<const Rendexpr *, int> > exprStack;

    exprStack.push(std::make_pair(root.get(), 0));
    while (!exprStack.empty()) {
        std::pair<const Rendexpr *, int> &top = exprStack.top();
        if (const Rendexpr *child = renderContext.step(top.first, top.second++))
            exprStack.push(std::make_pair(child, 0));
        else {
            // You can debug output renderContext.imageStack.top() for top.first expression here
            exprStack.pop();
        }
    }
    return renderContext.finish();
}

PlacedImagePtr render(Renderer &renderer, ImageBase &imageBase, Component &component, const Rendexptr &root, double scale, const PixelBounds &bounds, double time, const std::function<void(const Rendexpr *, const PlacedImagePtr &)> &hook) {
    if (!root)
        return renderer.reframe(nullptr, bounds);

    RenderContext renderContext(renderer, imageBase, component, scale, bounds, time);
    std::stack<std::pair<const Rendexpr *, int> > exprStack;

    exprStack.push(std::make_pair(root.get(), 0));
    while (!exprStack.empty()) {
        std::pair<const Rendexpr *, int> &top = exprStack.top();
        if (const Rendexpr *child = renderContext.step(top.first, top.second++))
            exprStack.push(std::make_pair(child, 0));
        else {
            hook(top.first, renderContext.peek());
            exprStack.pop();
        }
    }
    return renderContext.finish();
}

}
