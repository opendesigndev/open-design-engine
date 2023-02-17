
#include "webp.h"

#ifdef ODE_MEDIA_WEBP_SUPPORT

#include <webp/decode.h>

namespace ode {

static std::vector<byte> readWholeFile(FILE *file) {
    ODE_ASSERT(file);
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    std::vector<byte> fileBytes;
    fileBytes.resize(fileSize);
    byte *dst = fileBytes.data(), *end = dst+fileBytes.size();
    for (size_t n, remaining = fileSize; dst < end && (n = fread(dst, 1, remaining, file)); dst += n, remaining -= n);
    ODE_ASSERT(dst-fileBytes.data() <= fileSize);
    ODE_ASSERT(dst == end);
    fileBytes.resize(dst-fileBytes.data());
    return fileBytes;
}

bool detectWebpFormat(const byte *data, size_t length) {
    return length >= 4 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F';
}

Bitmap loadWebp(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadWebp(file);
    return Bitmap();
}

Bitmap loadWebp(FILE *file) {
    std::vector<byte> fileBytes = readWholeFile(file);
    Vector2i dimensions;
    if (WebPGetInfo(fileBytes.data(), fileBytes.size(), &dimensions.x, &dimensions.y)) {
        Bitmap bitmap(PixelFormat::RGBA, dimensions);
        if (bitmap) {
            if (WebPDecodeRGBAInto(fileBytes.data(), fileBytes.size(), (byte *) bitmap, bitmap.size(), pixelSize(bitmap.format())*bitmap.width()))
                return bitmap;
        }
    }
    return Bitmap();
}

}

#endif
