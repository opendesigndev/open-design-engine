
#pragma once

#include <cstddef>
#include <memory>
#include <octopus/shape.h>
#include <ode/math/Matrix3x2.h>
#include <ode/geometry/Rectangle.h>
#include <ode/graphics/BitmapRef.h>

namespace ode {

/// Rasterizes vector shapes as raster images
class Rasterizer {

public:
    /// A preprocessed / optimized representation of a vector shape
    class Shape;

    /// Represents a pointer to a preprocessed vector shape
    class ShapePtr : public std::unique_ptr<Shape> {
    public:
        ShapePtr(std::nullptr_t = nullptr);
        ShapePtr(Shape *ptr);
        ShapePtr(ShapePtr &&orig);
        ~ShapePtr();
        ShapePtr &operator=(ShapePtr &&orig);
    };

    /// An OpenGL texture descriptor used for direct rasterization to texture
    struct TextureDescriptor {
        unsigned handle;
        PixelFormat format;
        Vector2i dimensions;
    };

    static constexpr int BODY = -1;

    Rasterizer();
    Rasterizer(const Rasterizer &) = delete;
    Rasterizer(Rasterizer &&orig);
    ~Rasterizer();
    Rasterizer &operator=(const Rasterizer &) = delete;
    Rasterizer &operator=(Rasterizer &&orig);

    /// Preprocesses and compiles an Octopus shape representation to a Rasterizer Shape pointer
    static ShapePtr createShape(const octopus::Shape &octopusShape, int flags = 0);
    /// Updates Rasterizer Shape with a new Octopus shape representation
    static bool modifyShape(Shape *shape, const octopus::Shape &octopusShape, int flags = 0);

    /// Returns the graphical bounds of the shape
    static Rectangle<double> getBounds(Shape *shape, int strokeIndex, const Matrix3x2d &transformation);
    /// Rasterizes the shape (or its stroke) into a bitmap
    bool rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, const BitmapRef &dstBitmap);
    /// Rasterizes the shape (or its stroke) into a texture (the texture must be initialized)
    bool rasterize(Shape *shape, int strokeIndex, const Matrix3x2d &transformation, const TextureDescriptor &dstTexture);

private:
    class Internal;
    std::unique_ptr<Internal> data;

};

}
