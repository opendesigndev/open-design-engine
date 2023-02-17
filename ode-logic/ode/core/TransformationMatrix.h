
#pragma once

#include <ode-essentials.h>

namespace ode {

/// A 2D affine transformation matrix represented by a 3x2 matrix, behaves as a 3x3 matrix whose last row is always 0 0 1
class TransformationMatrix : public Matrix3x2d {

public:
    static const TransformationMatrix identity;

    constexpr TransformationMatrix() : Matrix3x2d(1) { }
    constexpr TransformationMatrix(const double values[3][2]) : Matrix3x2d(values) { }
    constexpr explicit TransformationMatrix(const double v[6]) : Matrix3x2d(v) { }
    constexpr explicit TransformationMatrix(const Matrix3x2d &orig) : Matrix3x2d(orig) { }
    constexpr TransformationMatrix(double m00, double m01, double m10, double m11, double m20, double m21) : Matrix3x2d(m00, m01, m10, m11, m20, m21) { }

    constexpr TransformationMatrix &operator*=(const TransformationMatrix &other) {
        return *this = *this*other;
    }

    friend constexpr TransformationMatrix operator*(const TransformationMatrix &a, const TransformationMatrix &b) {
        return TransformationMatrix(
            a[0][0]*b[0][0] + a[1][0]*b[0][1],
            a[0][1]*b[0][0] + a[1][1]*b[0][1],
            a[0][0]*b[1][0] + a[1][0]*b[1][1],
            a[0][1]*b[1][0] + a[1][1]*b[1][1],
            a[0][0]*b[2][0] + a[1][0]*b[2][1] + a[2][0],
            a[0][1]*b[2][0] + a[1][1]*b[2][1] + a[2][1]
        );
    }

    /// Generates a translation matrix
    static constexpr TransformationMatrix translate(const Vector2d &vector) {
        return TransformationMatrix(1, 0, 0, 1, vector.x, vector.y);
    }

    /// Generates a scale matrix (origin at 0, 0)
    static constexpr TransformationMatrix scale(double s) {
        return TransformationMatrix(s, 0, 0, s, 0, 0);
    }

    /// Generates a scale matrix (origin at 0, 0)
    static constexpr TransformationMatrix scale(const Vector2d &s) {
        return TransformationMatrix(s.x, 0, 0, s.y, 0, 0);
    }

    /// Generates a scale matrix with explicit origin (origin is the invariant point)
    static constexpr TransformationMatrix scale(const Vector2d &s, const Vector2d &origin) {
        return TransformationMatrix(s.x, 0, 0, s.y, (1-s.x)*origin.x, (1-s.y)*origin.y);
    }

    /// Generates a rotation matrix (center of rotation at 0, 0)
    static TransformationMatrix rotate(double a);

};

constexpr TransformationMatrix inverse(const TransformationMatrix &matrix) {
    return TransformationMatrix(Matrix3x2d(inverse(Matrix3x3d(matrix))));
}

}
