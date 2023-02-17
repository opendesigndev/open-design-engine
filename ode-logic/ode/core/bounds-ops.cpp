
#include "bounds-ops.h"

namespace ode {

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
