
#pragma once

#include <cstddef>
#include "../utils.h"
#include "../math/Vector2.h"
#include "../geometry/Rectangle.h"
#include "pixel-format.h"

namespace ode {

struct BitmapRef;
struct SparseBitmapConstRef;

/// A referemce to a mutable bitmap in memory with padding between rows
struct SparseBitmapRef {
    PixelFormat format;
    void *pixels;
    Vector2i dimensions;
    /// The difference between the beginnings of consecutive rows of pixels (in bytes)
    ptrdiff_t stride;

    constexpr SparseBitmapRef(std::nullptr_t = nullptr);
    constexpr SparseBitmapRef(PixelFormat format, void *pixels, const Vector2i &dimensions, ptrdiff_t stride);
    constexpr SparseBitmapRef(PixelFormat format, void *pixels, int width, int height, ptrdiff_t stride);
    constexpr SparseBitmapRef(const BitmapRef &orig);
    void clear() const;
    constexpr int width() const;
    constexpr int height() const;
    constexpr bool empty() const;
    constexpr explicit operator bool() const;
    constexpr explicit operator void *() const;
    explicit operator byte *() const;
    explicit operator float *() const;
    void * operator()(const Vector2i &position) const;
    void * operator()(int x, int y) const;
    /// Returns a reference to the portion of the bitmap denoted by area
    SparseBitmapRef subBitmap(const Rectangle<int> &area) const;
    /// Returns a reference to the bitmap flipped upside down
    SparseBitmapRef verticallyFlipped() const;

};

/// This variant can be used to overload functions with BitmapConstRef arguments where SparseBitmapConstRef would cause ambiguous conversions
struct SparseOnlyBitmapRef : SparseBitmapRef {
    constexpr SparseOnlyBitmapRef(std::nullptr_t = nullptr) { }
    constexpr SparseOnlyBitmapRef(const SparseBitmapRef &orig) : SparseBitmapRef(orig) { }
};

/// Copies pixels from src bitmap to dst, format and dimensions must equal
void copyPixels(const SparseBitmapRef &dst, const SparseBitmapConstRef &src);

}

#include "SparseBitmapRef.hpp"
