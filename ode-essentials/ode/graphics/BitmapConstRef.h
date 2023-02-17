
#pragma once

#include <cstddef>
#include "../utils.h"
#include "../math/Vector2.h"
#include "../geometry/Rectangle.h"
#include "pixel-format.h"

namespace ode {

struct BitmapRef;
struct SparseBitmapConstRef;

/// A reference to an immutable bitmap in contiguous memory
struct BitmapConstRef {
    PixelFormat format;
    const void *pixels;
    Vector2i dimensions;

    constexpr BitmapConstRef(std::nullptr_t = nullptr);
    constexpr BitmapConstRef(PixelFormat format, const void *pixels, const Vector2i &dimensions);
    constexpr BitmapConstRef(PixelFormat format, const void *pixels, int width, int height);
    constexpr BitmapConstRef(const BitmapRef &orig);
    constexpr int width() const;
    constexpr int height() const;
    constexpr size_t size() const;
    constexpr bool empty() const;
    constexpr explicit operator bool() const;
    constexpr explicit operator const void *() const;
    explicit operator const byte *() const;
    explicit operator const float *() const;
    const void *operator()(const Vector2i &position) const;
    const void *operator()(int x, int y) const;
    /// Returns a reference to the portion of the bitmap denoted by area
    SparseBitmapConstRef subBitmap(const Rectangle<int> &area) const;
    /// Returns a reference to the bitmap flipped upside down
    SparseBitmapConstRef verticallyFlipped() const;

};

}

#include "BitmapConstRef.hpp"
