
#pragma once

#include <cstddef>
#include "../utils.h"
#include "../math/Vector2.h"
#include "../geometry/Rectangle.h"
#include "pixel-format.h"

namespace ode {

struct BitmapRef;
struct BitmapConstRef;
struct SparseBitmapRef;

/// A referemce to an immutable bitmap in memory with padding between rows
struct SparseBitmapConstRef {
    PixelFormat format;
    const void *pixels;
    Vector2i dimensions;
    /// The difference between the beginnings of consecutive rows of pixels (in bytes)
    ptrdiff_t stride;

    constexpr SparseBitmapConstRef(std::nullptr_t = nullptr);
    constexpr SparseBitmapConstRef(PixelFormat format, const void *pixels, const Vector2i &dimensions, ptrdiff_t stride);
    constexpr SparseBitmapConstRef(PixelFormat format, const void *pixels, int width, int height, ptrdiff_t stride);
    constexpr SparseBitmapConstRef(const BitmapRef &orig);
    constexpr SparseBitmapConstRef(const BitmapConstRef &orig);
    constexpr SparseBitmapConstRef(const SparseBitmapRef &orig);
    constexpr int width() const;
    constexpr int height() const;
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

/// This variant can be used to overload functions with BitmapConstRef arguments where SparseBitmapConstRef would cause ambiguous conversions
struct SparseOnlyBitmapConstRef : SparseBitmapConstRef {
    constexpr SparseOnlyBitmapConstRef(std::nullptr_t = nullptr) { }
    constexpr SparseOnlyBitmapConstRef(const SparseBitmapRef &orig) : SparseBitmapConstRef(orig) { }
    constexpr SparseOnlyBitmapConstRef(const SparseBitmapConstRef &orig) : SparseBitmapConstRef(orig) { }
};

bool operator==(const SparseBitmapConstRef &a, const SparseBitmapConstRef &b);
bool operator!=(const SparseBitmapConstRef &a, const SparseBitmapConstRef &b);

}

#include "SparseBitmapConstRef.hpp"
