
#include "Bitmap.h"

#include <cstdlib>
#include <cstring>
#include "BitmapRef.h"
#include "BitmapConstRef.h"
#include "SparseBitmapRef.h"
#include "SparseBitmapConstRef.h"

namespace ode {

Bitmap::Bitmap(const BitmapRef &bitmap) : Bitmap(BitmapConstRef(bitmap)) { }

Bitmap::Bitmap(const BitmapConstRef &bitmap) : fmt(bitmap.format), data(), dims(bitmap.dimensions) {
    ODE_ASSERT(pixelSize(fmt) >= 0 && dims.x >= 0 && dims.y >= 0);
    if (bitmap && (data = malloc(size())))
        memcpy(data, bitmap.pixels, size());
    ODE_ASSERT(data || !bitmap);
}

Bitmap::Bitmap(const SparseBitmapConstRef &bitmap) : fmt(bitmap.format), data(), dims(bitmap.dimensions) {
    ODE_ASSERT(pixelSize(fmt) >= 0 && dims.x >= 0 && dims.y >= 0);
    if (bitmap && (data = malloc(size()))) {
        size_t rowSize = pixelSize(fmt)*dims.x;
        byte *dst = reinterpret_cast<byte *>(data);
        const byte *src = (const byte *) bitmap;
        for (int y = 0; y < bitmap.dimensions.y; ++y) {
            memcpy(dst, src, rowSize);
            dst += rowSize;
            src += bitmap.stride;
        }
    }
    ODE_ASSERT(data || !bitmap);
}

Bitmap::Bitmap(PixelFormat format, const Vector2i &dimensions) : fmt(format), data(), dims(dimensions) {
    ODE_ASSERT(pixelSize(fmt) >= 0 && dims.x >= 0 && dims.y >= 0);
    data = malloc(size());
    ODE_ASSERT(data || !size());
}

Bitmap::Bitmap(PixelFormat format, int width, int height) : Bitmap(format, Vector2i(width, height)) { }

Bitmap::Bitmap(PixelFormat format, const void *pixels, const Vector2i &dimensions) : fmt(format), data(), dims(dimensions) {
    ODE_ASSERT(pixelSize(fmt) >= 0 && dims.x >= 0 && dims.y >= 0 && pixels);
    if ((data = malloc(size())))
        memcpy(data, pixels, size());
    ODE_ASSERT(data || !size());
}

Bitmap::Bitmap(PixelFormat format, const void *pixels, int width, int height) : Bitmap(format, pixels, Vector2i(width, height)) { }

Bitmap::Bitmap(const Bitmap &orig) : fmt(orig.fmt), data(), dims(orig.dims) {
    if ((data = malloc(size())))
        memcpy(data, orig.data, size());
    ODE_ASSERT(data || !size());
}

Bitmap::Bitmap(Bitmap &&orig) : fmt(orig.fmt), data(orig.data), dims(orig.dims) {
    orig.data = nullptr;
    orig.dims = Vector2i();
}

Bitmap::~Bitmap() {
    free(data);
}

Bitmap &Bitmap::operator=(const Bitmap &orig) {
    if (this != &orig) {
        fmt = orig.fmt;
        dims = orig.dims;
        if (void *newData = realloc(data, size())) {
            data = newData;
            memcpy(data, orig.data, size());
        }
        ODE_ASSERT(data || !size());
    }
    return *this;
}

Bitmap &Bitmap::operator=(Bitmap &&orig) {
    if (this != &orig) {
        fmt = orig.fmt;
        data = orig.data;
        dims = orig.dims;
        orig.data = nullptr;
        orig.dims = Vector2i();
    }
    return *this;
}

Bitmap &Bitmap::operator=(const BitmapRef &bitmap) {
    return operator=(BitmapConstRef(bitmap));
}

Bitmap &Bitmap::operator=(const BitmapConstRef &bitmap) {
    ODE_ASSERT(pixelSize(bitmap.format) >= 0 && bitmap.dimensions.x >= 0 && bitmap.dimensions.y >= 0);
    fmt = bitmap.format;
    dims = bitmap.dimensions;
    if (void *newData = realloc(data, size())) {
        data = newData;
        memcpy(data, bitmap.pixels, size());
    }
    ODE_ASSERT(data || !size());
    return *this;
}

Bitmap &Bitmap::operator=(const SparseBitmapConstRef &bitmap) {
    ODE_ASSERT(pixelSize(bitmap.format) >= 0 && bitmap.dimensions.x >= 0 && bitmap.dimensions.y >= 0);
    if ((const byte *) bitmap >= reinterpret_cast<const byte *>(data) && (const byte *) bitmap < reinterpret_cast<const byte *>(data)+size()) // contains current data
        return operator=(Bitmap(bitmap));
    fmt = bitmap.format;
    dims = bitmap.dimensions;
    if (void *newData = realloc(data, size())) {
        data = newData;
        size_t rowSize = pixelSize(fmt)*dims.x;
        byte *dst = reinterpret_cast<byte *>(data);
        const byte *src = (const byte *) bitmap;
        for (int y = 0; y < bitmap.dimensions.y; ++y) {
            memcpy(dst, src, rowSize);
            dst += rowSize;
            src += bitmap.stride;
        }
    }
    return *this;
}

void Bitmap::clear() {
    memset(reinterpret_cast<byte *>(data), 0, size());
}

Bitmap::operator byte *() {
    return reinterpret_cast<byte *>(data);
}

Bitmap::operator const byte *() const {
    return reinterpret_cast<const byte *>(data);
}

Bitmap::operator float *() {
    ODE_ASSERT(isPixelFloat(fmt));
    return reinterpret_cast<float *>(data);
}

Bitmap::operator const float *() const {
    ODE_ASSERT(isPixelFloat(fmt));
    return reinterpret_cast<const float *>(data);
}

void *Bitmap::operator()(const Vector2i &position) {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dims.x && position.y <= dims.y);
    return reinterpret_cast<byte *>(data)+pixelSize(fmt)*(size_t(dims.x)*position.y+position.x);
}

const void *Bitmap::operator()(const Vector2i &position) const {
    ODE_ASSERT(position.x >= 0 && position.y >= 0 && position.x <= dims.x && position.y <= dims.y);
    return reinterpret_cast<const byte *>(data)+pixelSize(fmt)*(size_t(dims.x)*position.y+position.x);
}

void *Bitmap::operator()(int x, int y) {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dims.x && y <= dims.y);
    return reinterpret_cast<byte *>(data)+pixelSize(fmt)*(size_t(dims.x)*y+x);
}

const void *Bitmap::operator()(int x, int y) const {
    ODE_ASSERT(x >= 0 && y >= 0 && x <= dims.x && y <= dims.y);
    return reinterpret_cast<const byte *>(data)+pixelSize(fmt)*(size_t(dims.x)*y+x);
}

SparseBitmapRef Bitmap::subBitmap(const Rectangle<int> &area) {
    return operator BitmapRef().subBitmap(area);
}

SparseBitmapConstRef Bitmap::subBitmap(const Rectangle<int> &area) const {
    return operator BitmapConstRef().subBitmap(area);
}

SparseBitmapRef Bitmap::verticallyFlipped() {
    return operator BitmapRef().verticallyFlipped();
}

SparseBitmapConstRef Bitmap::verticallyFlipped() const {
    return operator BitmapConstRef().verticallyFlipped();
}

void Bitmap::reinterpret(PixelFormat newFormat) {
    ODE_ASSERT(pixelSize(newFormat) == pixelSize(fmt));
    fmt = newFormat;
}

void *Bitmap::eject() {
    void *ptr = data;
    data = nullptr;
    dims = Vector2i();
    return ptr;
}

}
