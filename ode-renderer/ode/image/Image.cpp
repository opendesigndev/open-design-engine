
#include "Image.h"

namespace ode {

ImagePtr Image::fromBitmap(const BitmapPtr &bitmap, TransparencyMode transparencyMode, BorderMode borderMode) {
    if (!bitmap)
        return nullptr;
    return ImagePtr(new BitmapImage(bitmap, transparencyMode, borderMode));
}

ImagePtr Image::fromBitmap(const SparseBitmapConstRef &bitmap, TransparencyMode transparencyMode, BorderMode borderMode) {
    if (!bitmap)
        return nullptr;
    return ImagePtr(new BitmapImage(BitmapPtr(new Bitmap(bitmap)), transparencyMode, borderMode));
}

ImagePtr Image::fromBitmap(Bitmap &&bitmap, TransparencyMode transparencyMode, BorderMode borderMode) {
    return ImagePtr(new BitmapImage(BitmapPtr(new Bitmap((Bitmap &&) bitmap)), transparencyMode, borderMode));
}

ImagePtr Image::fromTexture(const TexturePtr &texture, TransparencyMode transparencyMode, BorderMode borderMode) {
    if (!texture)
        return nullptr;
    return ImagePtr(new TextureImage(texture, transparencyMode, borderMode));
}

}
