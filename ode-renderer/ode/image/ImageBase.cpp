
#include "ImageBase.h"

#ifndef __EMSCRIPTEN__
    #include <ode-media.h>
#endif
#include "../optimized-renderer/process-asset-bitmap.h"

namespace ode {

ImageBase::ImageBase(GraphicsContext &gc) {
    ODE_ASSERT(gc);
}

void ImageBase::setImageDirectory(const FilePath &path) {
    directory = path;
}

void ImageBase::add(const octopus::Image &ref, const ImagePtr &image) {
    if (image->transparencyMode() == Image::PREMULTIPLIED && image->borderMode() == Image::ONE_PIXEL_BORDER) {
        if (ImagePtr texImage = Image::fromTexture(image->asTexture(), Image::PREMULTIPLIED, Image::ONE_PIXEL_BORDER))
            images.insert(std::make_pair(ref.ref.value, texImage));
    } else if (BitmapPtr bitmapImage = image->asBitmap())
        add(ref, *bitmapImage);
    else
        ODE_ASSERT(!"Failed to add image asset!");
}

void ImageBase::add(const octopus::Image &ref, const BitmapConstRef &imageBitmap) {
    ImagePtr image = processAssetBitmap(imageBitmap);
    if (image)
        images.insert(std::make_pair(ref.ref.value, image));
}

ImagePtr ImageBase::get(const octopus::Image &ref) {
    std::map<std::string, ImagePtr>::const_iterator it = images.find(ref.ref.value);
    if (it != images.end())
        return it->second;
    #ifndef __EMSCRIPTEN__
        if (!directory.empty()) {
            if (Bitmap bitmap = loadImage(directory+ref.ref.value)) {
                ImagePtr image = processAssetBitmap((Bitmap &&) bitmap);
                if (image)
                    images.insert(std::make_pair(ref.ref.value, image));
                return image;
            }
        }
    #endif
    return nullptr;
}

}
