
#include "process-asset-bitmap.h"

#include <algorithm>
#include <vector>
#include <ode/graphics/gl.h>

namespace ode {

// Implementation in Texture2D.cpp
void convertPixelFormat(PixelFormat format, GLint &internalFormat, GLenum &pixelFormat, GLenum &pixelType);

static Bitmap convertToRGBA(const BitmapConstRef &bitmap) {
    Bitmap dst(isPixelPremultiplied(bitmap.format) ? PixelFormat::PREMULTIPLIED_RGBA : PixelFormat::RGBA, bitmap.dimensions);
    const byte *spx = (const byte *) bitmap, *sEnd = spx+bitmap.size();
    byte *dpx = (byte *) dst, *dEnd = dpx+dst.size();
    switch (bitmap.format) {
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
    if (!texture->initialize(nullptr, w+2, h+2, bitmap.format))
        return nullptr;
    GLint internalFormat = GL_INVALID_INDEX;
    GLenum pixelFormat = GL_INVALID_INDEX;
    GLenum pixelType = GL_INVALID_INDEX;
    convertPixelFormat(bitmap.format, internalFormat, pixelFormat, pixelType);
    // Fill border
    std::vector<byte> borderPixels(pixelSize(bitmap.format)*(std::max(w, h)+2), (byte) 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w+2, 1, pixelFormat, pixelType, borderPixels.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, h+1, w+2, 1, pixelFormat, pixelType, borderPixels.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, h+2, pixelFormat, pixelType, borderPixels.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, w+1, 0, 1, h+2, pixelFormat, pixelType, borderPixels.data());
    // Fill center
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, w, h, pixelFormat, pixelType, bitmap.pixels);
    return Image::fromTexture((TexturePtr &&) texture, Image::PREMULTIPLIED, Image::ONE_PIXEL_BORDER);
}

ImagePtr processAssetBitmap(Bitmap &&bitmap) {
    // Ensure bitmap is premultiplied
    switch (bitmap.format()) {
        case PixelFormat::RGBA:
            for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 4) {
                p[0] = channelPremultiply(p[0], p[3]);
                p[1] = channelPremultiply(p[1], p[3]);
                p[2] = channelPremultiply(p[2], p[3]);
            }
            bitmap.reinterpret(PixelFormat::PREMULTIPLIED_RGBA);
            break;
        case PixelFormat::LUMINANCE_ALPHA:
            for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 2) {
                p[0] = channelPremultiply(p[0], p[1]);
            }
            bitmap.reinterpret(PixelFormat::PREMULTIPLIED_LUMINANCE_ALPHA);
            break;
        case PixelFormat::FLOAT_RGBA:
            for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 4) {
                p[0] *= p[3];
                p[1] *= p[3];
                p[2] *= p[3];
            }
            bitmap.reinterpret(PixelFormat::FLOAT_PREMULTIPLIED_RGBA);
            break;
        case PixelFormat::FLOAT_LUMINANCE_ALPHA:
            for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 2) {
                p[0] *= p[1];
            }
            bitmap.reinterpret(PixelFormat::FLOAT_PREMULTIPLIED_LUMINANCE_ALPHA);
            break;
        default:;
    }
    return processAssetBitmap(bitmap);
}

}
