
#pragma once

#include <functional>
#include <ode-logic.h>
#include "../image/Image.h"
#include "../image/ImageBase.h"
#include "Renderer.h"

namespace ode {

/// Renders the component
PlacedImagePtr render(Renderer &renderer, ImageBase &imageBase, Component &component, const Rendexptr &root, double scale, const PixelBounds &bounds, double time);

PlacedImagePtr render(Renderer &renderer, ImageBase &imageBase, Component &component, const Rendexptr &root, double scale, const PixelBounds &bounds, double time, const std::function<void(const Rendexpr *, const PlacedImagePtr &)> &hook);

}
