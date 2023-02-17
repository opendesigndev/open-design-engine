
#pragma once

#include <octopus/octopus.h>
#include <ode-essentials.h>
#include "TransformationMatrix.h"

namespace ode {

inline Vector2d fromOctopus(const octopus::Vec2 &vector) {
    return Vector2d(vector.x, vector.y);
}

inline octopus::Vec2 toOctopus(const Vector2d &vector) {
    octopus::Vec2 octopusVector;
    octopusVector.x = vector.x;
    octopusVector.y = vector.y;
    return octopusVector;
}

inline Color fromOctopus(const octopus::Color &color) {
    return Color(color.r, color.g, color.b, color.a);
}

inline octopus::Color toOctopus(const Color &color) {
    octopus::Color octopusColor;
    octopusColor.r = color.r;
    octopusColor.g = color.g;
    octopusColor.b = color.b;
    octopusColor.a = color.a;
    return octopusColor;
}

inline TransformationMatrix fromOctopusTransform(const double transform[6]) {
    return TransformationMatrix(transform);
}

inline void toOctopusTransform(double transform[6], const TransformationMatrix &matrix) {
    transform[0] = matrix[0][0];
    transform[1] = matrix[0][1];
    transform[2] = matrix[1][0];
    transform[3] = matrix[1][1];
    transform[4] = matrix[2][0];
    transform[5] = matrix[2][1];
}

}
