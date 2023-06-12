
#include "image-io.h"

namespace ode {

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

Bitmap loadImage(const byte *data, size_t length) {
    if (detectPngFormat(data, length))
        return loadPng(data, length);
    if (detectWebpFormat(data, length))
        return loadWebp(data, length);
    if (detectJpegFormat(data, length))
        return loadJpeg(data, length);
    if (detectGifFormat(data, length))
        return loadGif(data, length);
    // TODO: Support tiff
//    if (detectTiffFormat(data, length))
//        return loadTiff(data, length);
    if (detectRgbaFormat(data, length))
        return loadRgba(data, length);
    return Bitmap();
}

}
