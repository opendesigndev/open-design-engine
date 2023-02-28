
#include "bitmap-ops.h"

#include "pixel-format.h"

namespace ode {

void bitmapPremultiply(Bitmap &bitmap) {
    PixelFormat format = bitmap.format();
    if (!pixelHasAlpha(format))
        return;
    ODE_ASSERT(!isPixelPremultiplied(format));
    if (isPixelPremultiplied(format))
        return;
    if (isPixelFloat(format)) {
        switch (pixelChannels(format)) {
            case 2:
                ODE_ASSERT(format == PixelFormat::FLOAT_LUMINANCE_ALPHA);
                ODE_ASSERT(PixelFormat(int(format)|PIXEL_PREMULTIPLIED_BIT) == PixelFormat::FLOAT_PREMULTIPLIED_LUMINANCE_ALPHA);
                for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 2) {
                    p[0] *= p[1];
                }
                break;
            case 4:
                ODE_ASSERT(format == PixelFormat::FLOAT_RGBA);
                ODE_ASSERT(PixelFormat(int(format)|PIXEL_PREMULTIPLIED_BIT) == PixelFormat::FLOAT_PREMULTIPLIED_RGBA);
                for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 4) {
                    p[0] *= p[3];
                    p[1] *= p[3];
                    p[2] *= p[3];
                }
                break;
            default:
                ODE_ASSERT(!"Unknown pixel format");
                return;
        }
    } else {
        switch (pixelChannels(format)) {
            case 2:
                ODE_ASSERT(format == PixelFormat::LUMINANCE_ALPHA);
                ODE_ASSERT(PixelFormat(int(format)|PIXEL_PREMULTIPLIED_BIT) == PixelFormat::PREMULTIPLIED_LUMINANCE_ALPHA);
                for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 2) {
                    p[0] = channelPremultiply(p[0], p[1]);
                }
                break;
            case 4:
                ODE_ASSERT(format == PixelFormat::RGBA);
                ODE_ASSERT(PixelFormat(int(format)|PIXEL_PREMULTIPLIED_BIT) == PixelFormat::PREMULTIPLIED_RGBA);
                for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 4) {
                    p[0] = channelPremultiply(p[0], p[3]);
                    p[1] = channelPremultiply(p[1], p[3]);
                    p[2] = channelPremultiply(p[2], p[3]);
                }
                break;
            default:
                ODE_ASSERT(!"Unknown pixel format");
                return;
        }
    }
    bitmap.reinterpret(PixelFormat(int(format)|PIXEL_PREMULTIPLIED_BIT));
}

void bitmapUnpremultiply(Bitmap &bitmap) {
    PixelFormat format = bitmap.format();
    if (!pixelHasAlpha(format))
        return;
    ODE_ASSERT(isPixelPremultiplied(format));
    if (!isPixelPremultiplied(format))
        return;
    if (isPixelFloat(format)) {
        switch (pixelChannels(format)) {
            case 2:
                ODE_ASSERT(format == PixelFormat::FLOAT_PREMULTIPLIED_LUMINANCE_ALPHA);
                ODE_ASSERT(PixelFormat(int(format)&~PIXEL_PREMULTIPLIED_BIT) == PixelFormat::FLOAT_LUMINANCE_ALPHA);
                for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 2) {
                    if (p[1]) {
                        p[0] /= p[1];
                    }
                }
                break;
            case 4:
                ODE_ASSERT(format == PixelFormat::FLOAT_PREMULTIPLIED_RGBA);
                ODE_ASSERT(PixelFormat(int(format)&~PIXEL_PREMULTIPLIED_BIT) == PixelFormat::FLOAT_RGBA);
                for (float *p = (float *) bitmap, *end = p+bitmap.size()/sizeof(float); p < end; p += 4) {
                    if (p[3]) {
                        p[0] /= p[3];
                        p[1] /= p[3];
                        p[2] /= p[3];
                    }
                }
                break;
            default:
                ODE_ASSERT(!"Unknown pixel format");
                return;
        }
    } else {
        switch (pixelChannels(format)) {
            case 2:
                ODE_ASSERT(format == PixelFormat::PREMULTIPLIED_LUMINANCE_ALPHA);
                ODE_ASSERT(PixelFormat(int(format)&~PIXEL_PREMULTIPLIED_BIT) == PixelFormat::LUMINANCE_ALPHA);
                for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 2) {
                    p[0] = channelUnpremultiply(p[0], p[1]);
                }
                break;
            case 4:
                ODE_ASSERT(format == PixelFormat::PREMULTIPLIED_RGBA);
                ODE_ASSERT(PixelFormat(int(format)&~PIXEL_PREMULTIPLIED_BIT) == PixelFormat::RGBA);
                for (byte *p = (byte *) bitmap, *end = p+bitmap.size(); p < end; p += 4) {
                    p[0] = channelUnpremultiply(p[0], p[3]);
                    p[1] = channelUnpremultiply(p[1], p[3]);
                    p[2] = channelUnpremultiply(p[2], p[3]);
                }
                break;
            default:
                ODE_ASSERT(!"Unknown pixel format");
                return;
        }
    }
    bitmap.reinterpret(PixelFormat(int(format)&~PIXEL_PREMULTIPLIED_BIT));
}

}
