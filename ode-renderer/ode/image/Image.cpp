
#include "Image.h"

namespace ode {

ImagePtr Image::fromBitmap(const BitmapPtr &bitmap, TransparencyMode mode) {
    if (!bitmap)
        return nullptr;
    return ImagePtr(new BitmapImage(bitmap, mode));
}

ImagePtr Image::fromBitmap(const SparseBitmapConstRef &bitmap, TransparencyMode mode) {
    if (!bitmap)
        return nullptr;
    return ImagePtr(new BitmapImage(BitmapPtr(new Bitmap(bitmap)), mode));
}

ImagePtr Image::fromBitmap(Bitmap &&bitmap, TransparencyMode mode) {
    return ImagePtr(new BitmapImage(BitmapPtr(new Bitmap((Bitmap &&) bitmap)), mode));
}

ImagePtr Image::fromTexture(const TexturePtr &texture, TransparencyMode mode) {
    if (!texture)
        return nullptr;
    return ImagePtr(new TextureImage(texture, mode));
}

}
