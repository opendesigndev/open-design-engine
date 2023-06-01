
#include "rgba.h"

#include <cstdint>
#include <cstring>

namespace ode {

constexpr size_t HEADER_SIZE = 12;
constexpr size_t HEADER_SIGNATURE_SIZE = 4;
constexpr byte HEADER_SIGNATURE[] = "RGBA";

bool detectRgbaFormat(const byte *data, size_t length) {
    return length >= HEADER_SIZE && memcmp(data, HEADER_SIGNATURE, HEADER_SIGNATURE_SIZE) == 0;
}

Bitmap loadRgba(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadRgba(file);
    return Bitmap();
}

Bitmap loadRgba(FILE *file) {
    ODE_ASSERT(file);
    byte header[HEADER_SIZE];
    if (fread(header, 1, sizeof(header), file) == sizeof(header) && !memcmp(header, HEADER_SIGNATURE, HEADER_SIGNATURE_SIZE)) {
        Vector2i dimensions;
        dimensions.x = unsigned(header[4])<<24|unsigned(header[5])<<16|unsigned(header[6])<<8|unsigned(header[7]);
        dimensions.y = unsigned(header[8])<<24|unsigned(header[9])<<16|unsigned(header[10])<<8|unsigned(header[11]);
        Bitmap bitmap(PixelFormat::RGBA, dimensions);
        if (bitmap) {
            if (fread(bitmap.pixels(), 1, bitmap.size(), file) == bitmap.size())
                return bitmap;
        }
    }
    return Bitmap();
}

Bitmap loadRgba(const byte *data, size_t length) {
    ODE_ASSERT(data);
    if (detectRgbaFormat(data, length)) {
        Vector2i dimensions;
        dimensions.x = unsigned(data[4])<<24|unsigned(data[5])<<16|unsigned(data[6])<<8|unsigned(data[7]);
        dimensions.y = unsigned(data[8])<<24|unsigned(data[9])<<16|unsigned(data[10])<<8|unsigned(data[11]);
        Bitmap bitmap(PixelFormat::RGBA, dimensions);
        memcpy(bitmap.pixels(), data+HEADER_SIZE, length-HEADER_SIZE);
    }
    return Bitmap();
}

bool saveRgba(const FilePath &path, SparseBitmapConstRef bitmap) {
    ODE_ASSERT(bitmap.format == PixelFormat::RGBA);
    if (bitmap.format != PixelFormat::RGBA)
        return false;
    if (FilePtr file = openFile(path, true)) {
        byte header[HEADER_SIZE] = { 'R', 'G', 'B', 'A' };
        header[4] = byte(bitmap.dimensions.x>>24);
        header[5] = byte(bitmap.dimensions.x>>16);
        header[6] = byte(bitmap.dimensions.x>>8);
        header[7] = byte(bitmap.dimensions.x);
        header[8] = byte(bitmap.dimensions.y>>24);
        header[9] = byte(bitmap.dimensions.y>>16);
        header[10] = byte(bitmap.dimensions.y>>8);
        header[11] = byte(bitmap.dimensions.y);
        if (fwrite(header, 1, sizeof(header), file) != sizeof(header))
            return false;
        size_t rowSize = pixelSize(bitmap.format)*bitmap.dimensions.x;
        const byte *row = reinterpret_cast<const byte *>(bitmap.pixels);
        for (int y = 0; y < bitmap.dimensions.y; row += bitmap.stride, ++y) {
            if (fwrite(row, 1, rowSize, file) != rowSize)
                return false;
        }
        return true;
    }
    return false;
}

}
