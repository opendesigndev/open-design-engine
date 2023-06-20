
#include "gif.h"

#ifdef ODE_MEDIA_GIF_SUPPORT

#include <gif_lib.h>

namespace ode {

class GifFileGuard {
    GifFileType *gif;
public:
    inline explicit GifFileGuard(GifFileType *gif) : gif(gif) { }
    GifFileGuard(const GifFileGuard &) = delete;
    inline ~GifFileGuard() {
        DGifCloseFile(gif, NULL);
    }
};

struct ReadDataHandle {
    const void* data;
    const size_t size;
    size_t offset;
};

static Bitmap loadGif(GifFileType *gif) {
    GifFileGuard gifGuard(gif);
    if (DGifSlurp(gif) == GIF_OK && gif->ImageCount > 0) {
        const byte *src = gif->SavedImages[0].RasterBits;
        GifWord width = gif->SavedImages[0].ImageDesc.Width, height = gif->SavedImages[0].ImageDesc.Height;
        const ColorMapObject *colorMap = gif->SavedImages[0].ImageDesc.ColorMap;
        if (!colorMap)
            colorMap = gif->SColorMap;
        if (src && width > 0 && height > 0 && colorMap) {
            int transparent = -1;
            GraphicsControlBlock gcb;
            if (DGifSavedExtensionToGCB(gif, 0, &gcb) == GIF_OK)
                transparent = gcb.TransparentColor;
            Bitmap bitmap(PixelFormat::RGBA, width, height);
            if (bitmap) {
                byte *cur = (byte *) bitmap;
                int pixelCount = width*height;
                for (int i = 0; i < pixelCount; ++i) {
                    int index = *src++;
                    GifColorType c = colorMap->Colors[index];
                    *cur++ = c.Red;
                    *cur++ = c.Green;
                    *cur++ = c.Blue;
                    *cur++ = 0xff*(index != transparent);
                }
                return bitmap;
            }
        }
    }
    return Bitmap();
}


#ifndef __EMSCRIPTEN__
static int gifFileRead(GifFileType *context, GifByteType *data, int length) {
    FILE *file = reinterpret_cast<FILE *>(context->UserData);
    return (int) fread(data, 1, length, file);
}
#endif

static int gifMemoryRead(GifFileType* context, GifByteType* data, int length) {
    ReadDataHandle *dataHandle = reinterpret_cast<ReadDataHandle *>(context->UserData);
    memcpy(data, reinterpret_cast<const byte*>(dataHandle->data) + dataHandle->offset, length);
    dataHandle->offset += length;
    return length;
}

bool detectGifFormat(const byte *data, size_t length) {
    return length >= 3 && data[0] == 'G' && data[1] == 'I' && data[2] == 'F';
}

#ifndef __EMSCRIPTEN__
Bitmap loadGif(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadGif(file);
    return Bitmap();
}
#endif

#ifndef __EMSCRIPTEN__
Bitmap loadGif(FILE *file) {
    ODE_ASSERT(file);
    if (GifFileType *gif = DGifOpen(file, gifFileRead, NULL)) {
        return loadGif(gif);
    }
    return Bitmap();
}
#endif

Bitmap loadGif(const byte *data, size_t length) {
    ODE_ASSERT(data);
    ODE_ASSERT(length > 0);
    ReadDataHandle dataHandle { data, length, 0 };
    if (GifFileType *gif = DGifOpen(reinterpret_cast<void*>(&dataHandle), gifMemoryRead, nullptr)) {
        return loadGif(gif);
    }
    return Bitmap();
}

}

#endif
