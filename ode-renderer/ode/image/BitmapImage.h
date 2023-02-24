
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include "Image.h"

namespace ode {

/// An image stored as a bitmap in physical memory
class BitmapImage : public Image {

public:
    BitmapImage(const BitmapPtr &bitmap, TransparencyMode transparencyMode, BorderMode borderMode);
    virtual BitmapPtr asBitmap() const override;
    virtual TexturePtr asTexture() const override;
    virtual Vector2i dimensions() const override;

private:
    BitmapPtr bitmap;

};

}
