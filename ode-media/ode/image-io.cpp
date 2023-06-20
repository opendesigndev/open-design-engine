
#include "image-io.h"

namespace ode {

#ifndef __EMSCRIPTEN__
Bitmap loadImage(const FilePath &path) {
    if (FilePtr file = openFile(path, false)) {
        byte buffer[16] = { };
        fread(buffer, 1, sizeof(buffer), file);
        rewind(file);
        if (detectPngFormat(buffer, sizeof(buffer)))
            return loadPng(file);
        if (detectWebpFormat(buffer, sizeof(buffer)))
            return loadWebp(file);
        if (detectJpegFormat(buffer, sizeof(buffer)))
            return loadJpeg(file);
        if (detectGifFormat(buffer, sizeof(buffer)))
            return loadGif(file);
        if (detectTiffFormat(buffer, sizeof(buffer)))
            return loadTiff(file);
        if (detectRgbaFormat(buffer, sizeof(buffer)))
            return loadRgba(file);
    }
    return Bitmap();
}
#endif

Bitmap loadImage(const byte *data, size_t length) {
    if (detectPngFormat(data, length))
        return loadPng(data, length);
    if (detectJpegFormat(data, length))
        return loadJpeg(data, length);
#ifndef __EMSCRIPTEN__
    if (detectWebpFormat(data, length))
        return loadWebp(data, length);
    if (detectGifFormat(data, length))
        return loadGif(data, length);
    if (detectTiffFormat(data, length))
        return loadTiff(data, length);
#endif
    if (detectRgbaFormat(data, length))
        return loadRgba(data, length);
    return Bitmap();
}

}
