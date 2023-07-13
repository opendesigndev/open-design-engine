
#include "planar-intersections.h"

#include <algorithm>

namespace ode {

static bool convexPolygonCircleIntersection(const Vector2d *vertices, int sides, const Vector2d &center, double radius) {
    // ADAPTED FROM ARTERY ENGINE
    bool cInside = true;
    for (int i = 0; i < sides; ++i) {
        Vector2d ic = center-vertices[i];
        Vector2d edge = vertices[i+1]-vertices[i];
        if ((ic-std::min(std::max((dotProduct(ic, edge)/dotProduct(edge, edge)), 0.), 1.)*edge).squaredLength() <= radius*radius)
            return true;
        if (cInside && crossProduct(ic, edge) > 0)
            cInside = false;
    }
    return cInside;
}

bool intersects(const UntransformedBounds &bounds, const TransformationMatrix &transformation, const Vector2d &center, double radius) {
    Vector2d polygon[5] = {
        bounds.a,
        Vector2d(bounds.b.x, bounds.a.y),
        bounds.b,
        Vector2d(bounds.a.x, bounds.b.y),
        bounds.a
    };
    if (transformation[0][0]*transformation[1][1] < transformation[0][1]*transformation[1][0]) { // negative determinant - inversion transform
        std::swap(polygon[1], polygon[3]);
    }
    for (Vector2d &v : polygon)
        v = transformation*Vector3d(v.x, v.y, 1);
    return convexPolygonCircleIntersection(polygon, 4, center, radius);
}

}
