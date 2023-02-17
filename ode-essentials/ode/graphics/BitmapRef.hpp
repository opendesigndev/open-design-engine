
#include "BitmapRef.h"

#include <cstring>
#include "SparseBitmapRef.h"

namespace ode {

constexpr BitmapRef::BitmapRef(std::nullptr_t) : format(), pixels(nullptr) { }

constexpr BitmapRef::BitmapRef(PixelFormat format, void *pixels, const Vector2i &dimensions) : format(format), pixels(pixels), dimensions(dimensions) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !size());
}

constexpr BitmapRef::BitmapRef(PixelFormat format, void *pixels, int width, int height) : format(format), pixels(pixels), dimensions(width, height) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !size());
}

inline void BitmapRef::clear() const {
    memset(pixels, 0, size());
}

constexpr int BitmapRef::width() const {
    return dimensions.x;
}

constexpr int BitmapRef::height() const {
    return dimensions.y;
}

constexpr size_t BitmapRef::size() const {
    return pixelSize(format)*dimensions.x*dimensions.y;
}

constexpr bool BitmapRef::empty() const {
    return !(format != PixelFormat::EMPTY && pixels && dimensions.x && dimensions.y);
}

constexpr BitmapRef::operator bool() const {
    return pixels != nullptr;
}

constexpr BitmapRef::operator void *() const {
    return pixels;
}

inline BitmapRef::operator byte *() const {
    return reinterpret_cast<byte *>(pixels);
}

inline BitmapRef::operator float *() const {
    ODE_ASSERT(isPixelFloat(format));
    return reinterpret_cast<float *>(pixels);
}

inline void *BitmapRef::operator()(const Vector2i &position) const {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dimensions.x && position.y <= dimensions.y);
    return reinterpret_cast<byte *>(pixels)+pixelSize(format)*(size_t(dimensions.x)*position.y+position.x);
}

inline void *BitmapRef::operator()(int x, int y) const {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dimensions.x && y <= dimensions.y);
    return reinterpret_cast<byte *>(pixels)+pixelSize(format)*(size_t(dimensions.x)*y+x);
}

inline SparseBitmapRef BitmapRef::subBitmap(const Rectangle<int> &area) const {
    ODE_ASSERT(area.a.x >= 0 && area.a.y >= 0 && area.b.x <= dimensions.x && area.b.y <= dimensions.y && area.a.x <= area.b.x && area.a.y <= area.b.y);
    return SparseBitmapRef(format, operator()(area.a), area.b-area.a, pixelSize(format)*dimensions.x);
}

inline SparseBitmapRef BitmapRef::verticallyFlipped() const {
    if (dimensions.y <= 1)
        return SparseBitmapRef(*this);
    return SparseBitmapRef(format, operator()(0, dimensions.y-1), dimensions, -ptrdiff_t(pixelSize(format)*dimensions.x));
}

}
