
#include "SparseBitmapRef.h"

#include <cstring>
#include <algorithm>
#include "BitmapRef.h"
#include "SparseBitmapConstRef.h"

namespace ode {

constexpr SparseBitmapRef::SparseBitmapRef(std::nullptr_t) : format(), pixels(nullptr), stride(0) { }

constexpr SparseBitmapRef::SparseBitmapRef(PixelFormat format, void *pixels, const Vector2i &dimensions, ptrdiff_t stride) : format(format), pixels(pixels), dimensions(dimensions), stride(stride) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !pixelSize(format) || !dimensions.x || !dimensions.y);
}

constexpr SparseBitmapRef::SparseBitmapRef(PixelFormat format, void *pixels, int width, int height, ptrdiff_t stride) : format(format), pixels(pixels), dimensions(width, height), stride(stride) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !pixelSize(format) || !dimensions.x || !dimensions.y);
}

constexpr SparseBitmapRef::SparseBitmapRef(const BitmapRef &orig) : format(orig.format), pixels(orig.pixels), dimensions(orig.dimensions), stride(pixelSize(orig.format)*orig.dimensions.x) { }

inline void SparseBitmapRef::clear() const {
    size_t rowSize = pixelSize(format)*dimensions.x;
    byte *dst = reinterpret_cast<byte *>(pixels);
    for (int y = 0; y < dimensions.y; ++y) {
        memset(dst, 0, rowSize);
        dst += stride;
    }
}

constexpr int SparseBitmapRef::width() const {
    return dimensions.x;
}

constexpr int SparseBitmapRef::height() const {
    return dimensions.y;
}

constexpr bool SparseBitmapRef::empty() const {
    return !(format != PixelFormat::EMPTY && pixels && dimensions.x && dimensions.y);
}

constexpr SparseBitmapRef::operator bool() const {
    return pixels != nullptr;
}

constexpr SparseBitmapRef::operator void *() const {
    return pixels;
}

inline SparseBitmapRef::operator byte *() const {
    return reinterpret_cast<byte *>(pixels);
}

inline SparseBitmapRef::operator float *() const {
    ODE_ASSERT(isPixelFloat(format));
    return reinterpret_cast<float *>(pixels);
}

inline void *SparseBitmapRef::operator()(const Vector2i &position) const {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dimensions.x && position.y <= dimensions.y);
    return reinterpret_cast<byte *>(pixels)+stride*position.y+pixelSize(format)*position.x;
}

inline void *SparseBitmapRef::operator()(int x, int y) const {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dimensions.x && y <= dimensions.y);
    return reinterpret_cast<byte *>(pixels)+stride*y+pixelSize(format)*x;
}

inline SparseBitmapRef SparseBitmapRef::subBitmap(const Rectangle<int> &area) const {
    ODE_ASSERT(area.a.x >= 0 && area.a.y >= 0 && area.b.x <= dimensions.x && area.b.y <= dimensions.y && area.a.x <= area.b.x && area.a.y <= area.b.y);
    return SparseBitmapRef(format, operator()(area.a), area.b-area.a, stride);
}

inline SparseBitmapRef SparseBitmapRef::verticallyFlipped() const {
    if (dimensions.y <= 1)
        return SparseBitmapRef(*this);
    return SparseBitmapRef(format, operator()(0, dimensions.y-1), dimensions, -stride);
}

inline void copyPixels(const SparseBitmapRef &dst, const SparseBitmapConstRef &src) {
    ODE_ASSERT(dst.format == src.format && !dst.pixels == !src.pixels);
    Vector2i dims(std::min(dst.dimensions.x, src.dimensions.x), std::min(dst.dimensions.y, src.dimensions.y));
    size_t rowSize = std::min(pixelSize(dst.format), pixelSize(src.format))*dims.x;
    byte *dstp = (byte *) dst;
    const byte *srcp = (const byte *) src;
    for (int y = 0; y < dims[1]; ++y) {
        memcpy(dstp, srcp, rowSize);
        dstp += dst.stride;
        srcp += src.stride;
    }
}

}
