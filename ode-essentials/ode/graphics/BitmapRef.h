
#pragma once

#include <cstddef>
#include "../utils.h"
#include "../math/Vector2.h"
#include "../geometry/Rectangle.h"
#include "pixel-format.h"

namespace ode {

struct SparseBitmapRef;

/// A reference to a mutable bitmap in contiguous memory
struct BitmapRef {
    PixelFormat format;
    void *pixels;
    Vector2i dimensions;

    constexpr BitmapRef(std::nullptr_t = nullptr);
    constexpr BitmapRef(PixelFormat format, void *pixels, const Vector2i &dimensions);
    constexpr BitmapRef(PixelFormat format, void *pixels, int width, int height);
    void clear() const;
    constexpr int width() const;
    constexpr int height() const;
    constexpr size_t size() const;
    constexpr bool empty() const;
    constexpr explicit operator bool() const;
    constexpr explicit operator void *() const;
    explicit operator byte *() const;
    explicit operator float *() const;
    void *operator()(const Vector2i &position) const;
    void *operator()(int x, int y) const;
    /// Returns a reference to the portion of the bitmap denoted by area
    SparseBitmapRef subBitmap(const Rectangle<int> &area) const;
    /// Returns a reference to the bitmap flipped upside down
    SparseBitmapRef verticallyFlipped() const;

};

}

#include "BitmapRef.hpp"
