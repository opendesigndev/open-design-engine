
#include "TextureImage.h"

namespace ode {

TextureImage::TextureImage(const TexturePtr &texture, TransparencyMode transparencyMode, BorderMode borderMode) : Image(transparencyMode, borderMode), texture(texture) { }

BitmapPtr TextureImage::asBitmap() const {
    return BitmapPtr(new Bitmap(texture->download()));
}

TexturePtr TextureImage::asTexture() const {
    return texture;
}

Vector2i TextureImage::dimensions() const {
    return texture ? texture->dimensions() : Vector2i();
}

}
