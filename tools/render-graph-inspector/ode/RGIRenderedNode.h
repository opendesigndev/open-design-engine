
#pragma once

#include <ode-logic.h>

// Forward declarations
namespace ode {
class Bitmap;
using BitmapPtr = std::shared_ptr<Bitmap>;
}

using namespace ode;

struct RGIRenderedNode {
    /// Index in the render graph.
    int index = 0;
    /// The render expression at this node.
    const Rendexpr *node = nullptr;
    /// The name of the node (most likely to be the name of the render expression combined with the index).
    std::string name;
    /// Rendererd image.
    BitmapPtr bitmap = nullptr;
    /// Image placement.
    ScaledBounds placement;
};
using RGIRenderedNodes = std::vector<RGIRenderedNode>;
