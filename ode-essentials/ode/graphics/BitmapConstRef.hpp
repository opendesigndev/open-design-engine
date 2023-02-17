
#include "BitmapConstRef.h"

#include "BitmapRef.h"
#include "SparseBitmapConstRef.h"

namespace ode {

constexpr BitmapConstRef::BitmapConstRef(std::nullptr_t) : format(), pixels(nullptr) { }

constexpr BitmapConstRef::BitmapConstRef(PixelFormat format, const void *pixels, const Vector2i &dimensions) : format(format), pixels(pixels), dimensions(dimensions) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !size());
}

constexpr BitmapConstRef::BitmapConstRef(PixelFormat format, const void *pixels, int width, int height) : format(format), pixels(pixels), dimensions(width, height) {
    ODE_ASSERT(pixelSize(format) >= 0 && dimensions.x >= 0 && dimensions.y >= 0);
    ODE_ASSERT(pixels || !size());
}

constexpr BitmapConstRef::BitmapConstRef(const BitmapRef &orig) : format(orig.format), pixels(orig.pixels), dimensions(orig.dimensions) { }

constexpr int BitmapConstRef::width() const {
    return dimensions.x;
}

constexpr int BitmapConstRef::height() const {
    return dimensions.y;
}

constexpr size_t BitmapConstRef::size() const {
    return pixelSize(format)*dimensions.x*dimensions.y;
}

constexpr bool BitmapConstRef::empty() const {
    return !(format != PixelFormat::EMPTY && pixels && dimensions.x && dimensions.y);
}

constexpr BitmapConstRef::operator bool() const {
    return pixels != nullptr;
}

constexpr BitmapConstRef::operator const void *() const {
    return pixels;
}

inline BitmapConstRef::operator const byte *() const {
    return reinterpret_cast<const byte *>(pixels);
}

inline BitmapConstRef::operator const float *() const {
    ODE_ASSERT(isPixelFloat(format));
    return reinterpret_cast<const float *>(pixels);
}

inline const void *BitmapConstRef::operator()(const Vector2i &position) const {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dimensions.x && position.y <= dimensions.y);
    return reinterpret_cast<const byte *>(pixels)+pixelSize(format)*(size_t(dimensions.x)*position.y+position.x);
}

inline const void *BitmapConstRef::operator()(int x, int y) const {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dimensions.x && y <= dimensions.y);
    return reinterpret_cast<const byte *>(pixels)+pixelSize(format)*(size_t(dimensions.x)*y+x);
}

inline SparseBitmapConstRef BitmapConstRef::subBitmap(const Rectangle<int> &area) const {
    ODE_ASSERT(area.a.x >= 0 && area.a.y >= 0 && area.b.x <= dimensions.x && area.b.y <= dimensions.y && area.a.x <= area.b.x && area.a.y <= area.b.y);
    return SparseBitmapConstRef(format, operator()(area.a), area.b-area.a, pixelSize(format)*dimensions.x);
}

inline SparseBitmapConstRef BitmapConstRef::verticallyFlipped() const {
    if (dimensions.y <= 1)
        return SparseBitmapConstRef(*this);
    return SparseBitmapConstRef(format, operator()(0, dimensions.y-1), dimensions, -ptrdiff_t(pixelSize(format)*dimensions.x));
}

}
