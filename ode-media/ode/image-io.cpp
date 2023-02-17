
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

}
