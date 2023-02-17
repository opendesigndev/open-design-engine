
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
struct SparseBitmapConstRef;

/// Manages a raw image bitmap in contiguous memory
class Bitmap {

public:
    inline Bitmap() : fmt(), data(), dims() { }
    explicit Bitmap(const BitmapRef &bitmap);
    explicit Bitmap(const BitmapConstRef &bitmap);
    explicit Bitmap(const SparseBitmapConstRef &bitmap);
    Bitmap(PixelFormat format, const Vector2i &dimensions);
    Bitmap(PixelFormat format, int width, int height);
    Bitmap(PixelFormat format, const void *pixels, const Vector2i &dimensions);
    Bitmap(PixelFormat format, const void *pixels, int width, int height);
    Bitmap(const Bitmap &orig);
    Bitmap(Bitmap &&orig);
    ~Bitmap();
    Bitmap &operator=(const Bitmap &orig);
    Bitmap &operator=(Bitmap &&orig);
    Bitmap &operator=(const BitmapRef &bitmap);
    Bitmap &operator=(const BitmapConstRef &bitmap);
    Bitmap &operator=(const SparseBitmapConstRef &bitmap);
    /// Sets all pixel bytes to zero
    void clear();
    constexpr PixelFormat format() const;
    constexpr void * pixels();
    constexpr const void * pixels() const;
    constexpr Vector2i dimensions() const;
    constexpr int width() const;
    constexpr int height() const;
    /// Returns the memory footprint of the pixel array
    constexpr size_t size() const;
    /// Returns true if the bitmap has no pixels
    constexpr bool empty() const;
    constexpr operator BitmapRef();
    constexpr operator BitmapConstRef() const;
    constexpr operator SparseBitmapRef();
    constexpr operator SparseBitmapConstRef() const;
    /// Returns true if the bitmap contains any pixels
    constexpr explicit operator bool() const;
    /// Returns pointer to pixel data
    constexpr explicit operator void *();
    /// Returns const pointer to pixel data
    constexpr explicit operator const void *() const;
    /// Returns pointer to pixel data as bytes
    explicit operator byte *();
    /// Returns const pointer to pixel data as bytes
    explicit operator const byte *() const;
    /// Returns pointer to pixel data as floats - call only valid if bitmap has floating-point format
    explicit operator float *();
    /// Returns const pointer to pixel data as floats - call only valid if bitmap has floating-point format
    explicit operator const float *() const;
    /// Returns pointer to pixel at position
    void *operator()(const Vector2i &position);
    /// Returns const pointer to pixel at position
    const void *operator()(const Vector2i &position) const;
    /// Returns pointer to pixel at position x, y
    void *operator()(int x, int y);
    /// Returns const pointer to pixel at position x, y
    const void *operator()(int x, int y) const;
    /// Returns a reference to the portion of the bitmap denoted by area
    SparseBitmapRef subBitmap(const Rectangle<int> &area);
    /// Returns a constant reference to the portion of the bitmap denoted by area
    SparseBitmapConstRef subBitmap(const Rectangle<int> &area) const;
    /// Returns a reference to the bitmap flipped upside down
    SparseBitmapRef verticallyFlipped();
    /// Returns a constant reference to the bitmap flipped upside down
    SparseBitmapConstRef verticallyFlipped() const;
    /// Changes the pixel format without changing the data (size of old and new format must be equal!)
    void reinterpret(PixelFormat newFormat);
    /// Transfers memory management of the pixel array (return value) to the caller, use free() to deallocate
    void *eject();

private:
    PixelFormat fmt;
    void *data;
    Vector2i dims;

};

}

#include "Bitmap.hpp"
