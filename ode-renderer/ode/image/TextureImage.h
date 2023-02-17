
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include "Image.h"

namespace ode {

/// An image stored as an OpenGL texture
class TextureImage : public Image {

public:
    TextureImage(const TexturePtr &texture, TransparencyMode transparencyMode);
    virtual BitmapPtr asBitmap() const override;
    virtual TexturePtr asTexture() const override;
    virtual Vector2i dimensions() const override;

private:
    TexturePtr texture;

};

}
