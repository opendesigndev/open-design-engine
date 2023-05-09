
#include "process-asset-bitmap.h"

#include <cstring>
#include <algorithm>
#include <vector>
#include <ode-graphics.h>

namespace ode {

static Bitmap convertToRGBA(const BitmapConstRef &bitmap) {
    Bitmap dst(isPixelPremultiplied(bitmap.format) ? PixelFormat::PREMULTIPLIED_RGBA : PixelFormat::RGBA, bitmap.dimensions);
    const byte *spx = (const byte *) bitmap, *sEnd = spx+bitmap.size();
    byte *dpx = (byte *) dst, *dEnd = dpx+dst.size();
    switch (bitmap.format) {
        case PixelFormat::RGBA:
        case PixelFormat::PREMULTIPLIED_RGBA:
            memcpy(dpx, spx, dst.size());
            return dst;
        case PixelFormat::RGB:
            for (; dpx < dEnd; dpx += 4, spx += 3) {
                dpx[0] = spx[0];
                dpx[1] = spx[1];
                dpx[2] = spx[2];
                dpx[3] = byte(0xff);
            }
            break;
        case PixelFormat::R:
        case PixelFormat::LUMINANCE:
            for (; dpx < dEnd; dpx += 4, spx += 1) {
                dpx[0] = spx[0];
                dpx[1] = spx[0];
                dpx[2] = spx[0];
                dpx[3] = byte(0xff);
            }
            break;
        case PixelFormat::LUMINANCE_ALPHA:
        case PixelFormat::PREMULTIPLIED_LUMINANCE_ALPHA:
            for (; dpx < dEnd; dpx += 4, spx += 2) {
                dpx[0] = spx[0];
                dpx[1] = spx[0];
                dpx[2] = spx[0];
                dpx[3] = spx[1];
            }
            break;
        case PixelFormat::ALPHA:
            for (; dpx < dEnd; dpx += 4, spx += 1) {
                dpx[0] = byte(0);
                dpx[1] = byte(0);
                dpx[2] = byte(0);
                dpx[3] = spx[0];
            }
            break;
        default:
            ODE_ASSERT(!"Unexpected format");
    }
    ODE_ASSERT(dpx == dEnd && spx == sEnd);
    return dst;
}

ImagePtr processAssetBitmap(const BitmapConstRef &bitmap) {
    if (!bitmap)
        return nullptr;
    if (bitmap.format != PixelFormat::PREMULTIPLIED_RGBA)
        return processAssetBitmap(convertToRGBA(bitmap));
    int w = bitmap.width(), h = bitmap.height();
    // Create texture with 1 pixel transparent border
    TexturePtr texture(new Texture2D);
    if (!texture->initialize(bitmap.format, Vector2i(w+2, h+2)))
        return nullptr;
    std::vector<byte> borderPixels(pixelSize(bitmap.format)*(std::max(w, h)+2), (byte) 0);
    if (!(
        // Fill border
        texture->put(Vector2i(0, 0), BitmapConstRef(bitmap.format, borderPixels.data(), w+2, 1)) &&
        texture->put(Vector2i(0, h+1), BitmapConstRef(bitmap.format, borderPixels.data(), w+2, 1)) &&
        texture->put(Vector2i(0, 0), BitmapConstRef(bitmap.format, borderPixels.data(), 1, h+2)) &&
        texture->put(Vector2i(w+1, 0), BitmapConstRef(bitmap.format, borderPixels.data(), 1, h+2)) &&
        // Fill center
        texture->put(Vector2i(1, 1), BitmapConstRef(bitmap.format, bitmap.pixels, w, h))
    ))
        return nullptr;
    return Image::fromTexture((TexturePtr &&) texture, Image::PREMULTIPLIED, Image::ONE_PIXEL_BORDER);
}

ImagePtr processAssetBitmap(Bitmap &&bitmap) {
    if (!isPixelPremultiplied(bitmap.format()))
        bitmapPremultiply(bitmap);
    return processAssetBitmap(bitmap);
}

}
