
#include "rgba.h"

#include <cstdint>
#include <cstring>

namespace ode {

bool detectRgbaFormat(const byte *data, size_t length) {
    return length >= 4 && data[0] == 'R' && data[1] == 'G' && data[2] == 'B' && data[3] == 'A';
}

Bitmap loadRgba(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadRgba(file);
    return Bitmap();
}

Bitmap loadRgba(FILE *file) {
    ODE_ASSERT(file);
    byte header[12];
    if (fread(header, 1, sizeof(header), file) == sizeof(header) && !memcmp(header, "RGBA", 4)) {
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

bool saveRgba(const FilePath &path, SparseBitmapConstRef bitmap) {
    ODE_ASSERT(bitmap.format == PixelFormat::RGBA);
    if (bitmap.format != PixelFormat::RGBA)
        return false;
    if (FilePtr file = openFile(path, true)) {
        byte header[12] = { 'R', 'G', 'B', 'A' };
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
