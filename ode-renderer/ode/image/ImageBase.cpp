
#include "ImageBase.h"

#ifndef __EMSCRIPTEN__
    #include <ode-media.h>
#endif

namespace ode {

// TODO MOVE
static void ensurePremultiplied(Bitmap &bitmap) {
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
}

ImageBase::ImageBase(GraphicsContext &gc) {
    ODE_ASSERT(gc);
}

void ImageBase::setImageDirectory(const FilePath &path) {
    directory = path;
}

void ImageBase::add(const octopus::Image &ref, const ImagePtr &image) {
    if (ImagePtr texImage = Image::fromTexture(image->asTexture(), Image::NORMAL))
        images.insert(std::make_pair(ref.ref.value, texImage));
}

ImagePtr ImageBase::get(const octopus::Image &ref) {
    std::map<std::string, ImagePtr>::const_iterator it = images.find(ref.ref.value);
    if (it != images.end())
        return it->second;
    #ifndef __EMSCRIPTEN__
        if (!directory.empty()) {
            if (Bitmap bitmap = loadImage(directory+ref.ref.value)) {
                ensurePremultiplied(bitmap);
                TexturePtr texture(new Texture2D());
                if (texture->initialize(bitmap)) {
                    ImagePtr image = Image::fromTexture(texture, Image::NORMAL);
                    images.insert(std::make_pair(ref.ref.value, image));
                    return image;
                }
            }
        }
    #endif
    return nullptr;
}

}
