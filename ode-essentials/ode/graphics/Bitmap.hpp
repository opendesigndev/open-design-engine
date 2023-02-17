
#include "Bitmap.h"

#include "BitmapRef.h"
#include "BitmapConstRef.h"
#include "SparseBitmapRef.h"
#include "SparseBitmapConstRef.h"

namespace ode {

constexpr PixelFormat Bitmap::format() const {
    return fmt;
}

constexpr void * Bitmap::pixels() {
    return data;
}

constexpr const void * Bitmap::pixels() const {
    return data;
}

constexpr Vector2i Bitmap::dimensions() const {
    return dims;
}

constexpr int Bitmap::width() const {
    return dims.x;
}

constexpr int Bitmap::height() const {
    return dims.y;
}

constexpr size_t Bitmap::size() const {
    return pixelSize(fmt)*dims.x*dims.y;
}

constexpr bool Bitmap::empty() const {
    return !operator bool();
}

constexpr Bitmap::operator BitmapRef() {
    return BitmapRef(fmt, data, dims);
}

constexpr Bitmap::operator BitmapConstRef() const {
    return BitmapConstRef(fmt, data, dims);
}

constexpr Bitmap::operator SparseBitmapRef() {
    return SparseBitmapRef(fmt, data, dims, pixelSize(fmt)*dims.x);
}

constexpr Bitmap::operator SparseBitmapConstRef() const {
    return SparseBitmapConstRef(fmt, data, dims, pixelSize(fmt)*dims.x);
}

constexpr Bitmap::operator bool() const {
    return fmt != PixelFormat::EMPTY && dims.x && dims.y && data;
}

constexpr Bitmap::operator void *() {
    return data;
}

constexpr Bitmap::operator const void *() const {
    return data;
}

}
