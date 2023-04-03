
#include "bounds-ops.h"

#include <algorithm>

namespace ode {

UnscaledBounds transformBounds(const UntransformedBounds &bounds, const TransformationMatrix &matrix) {
    Vector2d a = matrix*Vector3d(bounds.a.x, bounds.a.y, 1);
    Vector2d b = matrix*Vector3d(bounds.b.x, bounds.a.y, 1);
    Vector2d c = matrix*Vector3d(bounds.a.x, bounds.b.y, 1);
    Vector2d d = matrix*Vector3d(bounds.b.x, bounds.b.y, 1);
    return UnscaledBounds(
        std::min(std::min(std::min(a.x, b.x), c.x), d.x),
        std::min(std::min(std::min(a.y, b.y), c.y), d.y),
        std::max(std::max(std::max(a.x, b.x), c.x), d.x),
        std::max(std::max(std::max(a.y, b.y), c.y), d.y)
    );
}

ScaledBounds scaleBounds(const UnscaledBounds &bounds, double scale) {
    return ScaledBounds(
        scale*bounds.a.x,
        scale*bounds.a.y,
        scale*bounds.b.x,
        scale*bounds.b.y
    );
}

PixelBounds outerPixelBounds(const ScaledBounds &bounds) {
    return PixelBounds(
        int(floor(bounds.a.x)),
        int(floor(bounds.a.y)),
        int(ceil(bounds.b.x)),
        int(ceil(bounds.b.y))
    );
}

PixelBounds innerPixelBounds(const ScaledBounds &bounds) {
    return PixelBounds(
        int(ceil(bounds.a.x)),
        int(ceil(bounds.a.y)),
        int(floor(bounds.b.x)),
        int(floor(bounds.b.y))
    );
}

}
