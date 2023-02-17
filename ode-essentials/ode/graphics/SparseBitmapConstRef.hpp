
#include "SparseBitmapConstRef.h"

#include <cstring>
#include "BitmapRef.h"
#include "BitmapConstRef.h"
#include "SparseBitmapRef.h"

namespace ode {

constexpr SparseBitmapConstRef::SparseBitmapConstRef(std::nullptr_t) : format(), pixels(nullptr), stride(0) { }

constexpr SparseBitmapConstRef::SparseBitmapConstRef(PixelFormat format, const void *pixels, const Vector2i &dimensions, ptrdiff_t stride) : format(format), pixels(pixels), dimensions(dimensions), stride(stride) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !pixelSize(format) || !dimensions.x || !dimensions.y);
}

constexpr SparseBitmapConstRef::SparseBitmapConstRef(PixelFormat format, const void *pixels, int width, int height, ptrdiff_t stride) : format(format), pixels(pixels), dimensions(width, height), stride(stride) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !pixelSize(format) || !dimensions.x || !dimensions.y);
}

constexpr SparseBitmapConstRef::SparseBitmapConstRef(const BitmapRef &orig) : SparseBitmapConstRef(BitmapConstRef(orig)) { }

constexpr SparseBitmapConstRef::SparseBitmapConstRef(const BitmapConstRef &orig) : format(orig.format), pixels(orig.pixels), dimensions(orig.dimensions), stride(pixelSize(orig.format)*orig.dimensions.x) { }

constexpr SparseBitmapConstRef::SparseBitmapConstRef(const SparseBitmapRef &orig) : format(orig.format), pixels(orig.pixels), dimensions(orig.dimensions), stride(orig.stride) { }

constexpr int SparseBitmapConstRef::width() const {
    return dimensions.x;
}

constexpr int SparseBitmapConstRef::height() const {
    return dimensions.y;
}

constexpr bool SparseBitmapConstRef::empty() const {
    return !(format != PixelFormat::EMPTY && pixels && dimensions.x && dimensions.y);
}

constexpr SparseBitmapConstRef::operator bool() const {
    return pixels != nullptr;
}

constexpr SparseBitmapConstRef::operator const void *() const {
    return pixels;
}

inline SparseBitmapConstRef::operator const byte *() const {
    return reinterpret_cast<const byte *>(pixels);
}

inline SparseBitmapConstRef::operator const float *() const {
    ODE_ASSERT(isPixelFloat(format));
    return reinterpret_cast<const float *>(pixels);
}

inline const void *SparseBitmapConstRef::operator()(const Vector2i &position) const {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dimensions.x && position.y <= dimensions.y);
    return reinterpret_cast<const byte *>(pixels)+stride*position.y+pixelSize(format)*position.x;
}

inline const void *SparseBitmapConstRef::operator()(int x, int y) const {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dimensions.x && y <= dimensions.y);
    return reinterpret_cast<const byte *>(pixels)+stride*y+pixelSize(format)*x;
}

inline SparseBitmapConstRef SparseBitmapConstRef::subBitmap(const Rectangle<int> &area) const {
    ODE_ASSERT(area.a.x >= 0 && area.a.y >= 0 && area.b.x <= dimensions.x && area.b.y <= dimensions.y && area.a.x <= area.b.x && area.a.y <= area.b.y);
    return SparseBitmapConstRef(format, operator()(area.a), area.b-area.a, stride);
}

inline SparseBitmapConstRef SparseBitmapConstRef::verticallyFlipped() const {
    if (dimensions.y <= 1)
        return SparseBitmapConstRef(*this);
    return SparseBitmapConstRef(format, operator()(0, dimensions.y-1), dimensions, -stride);
}

inline bool operator==(const SparseBitmapConstRef &a, const SparseBitmapConstRef &b) {
    if (!(a.format == b.format && a.dimensions == b.dimensions))
        return false;
    size_t rowSize = pixelSize(a.format)*a.dimensions.x;
    const byte *ap = (const byte *) a;
    const byte *bp = (const byte *) b;
    for (int y = 0; y < a.dimensions.y; ++y) {
        if (memcmp(ap, bp, rowSize))
            return false;
        ap += a.stride;
        bp += b.stride;
    }
    return true;
}

inline bool operator!=(const SparseBitmapConstRef &a, const SparseBitmapConstRef &b) {
    return !operator==(a, b);
}

}
