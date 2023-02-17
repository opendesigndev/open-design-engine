
#include "BitmapImage.h"

namespace ode {

BitmapImage::BitmapImage(const BitmapPtr &bitmap, TransparencyMode transparencyMode) : Image(transparencyMode), bitmap(bitmap) { }

BitmapPtr BitmapImage::asBitmap() const {
    return bitmap;
}

TexturePtr BitmapImage::asTexture() const {
    if (!bitmap)
        return nullptr;
    TexturePtr texture(new Texture2D);
    if (!texture->initialize(*bitmap))
        return nullptr;
    return texture;
}

Vector2i BitmapImage::dimensions() const {
    return bitmap ? bitmap->dimensions() : Vector2i();
}

}
