
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>

namespace ode {

class Image;

typedef std::shared_ptr<Image> ImagePtr;

typedef std::shared_ptr<Bitmap> BitmapPtr;
typedef std::shared_ptr<Texture2D> TexturePtr;

/// An abstract image with multiple possible underlying storage types
class Image {

public:
    enum TransparencyMode {
        /// Guaranteed to be fully opaque, all pixels have alpha at 100% (DO NOT USE IF THIS IS NOT CERTAIN) (can be treated as both premultiplied or unpremultiplied)
        NO_TRANSPARENCY = -1,
        /// Normal (unpremultiplied) representation of color channels and alpha
        NORMAL = 0,
        /// Color channels premultiplied by the alpha channel
        PREMULTIPLIED = 1,
        /// The content represents an alpha only image, which is encoded in the first (red) color channel
        RED_IS_ALPHA = 2
    };

    /// Constructs an Image from a bitmap
    static ImagePtr fromBitmap(const BitmapPtr &bitmap, TransparencyMode mode);
    /// Constructs an Image from a sparse bitmap
    static ImagePtr fromBitmap(const SparseBitmapConstRef &bitmap, TransparencyMode mode);
    /// Constructs an Image from a bitmap
    static ImagePtr fromBitmap(Bitmap &&bitmap, TransparencyMode mode);
    /// Constructs an Image from a texture
    static ImagePtr fromTexture(const TexturePtr &texture, TransparencyMode mode);

    Image(const Image &) = delete;
    virtual ~Image() = default;
    Image &operator=(const Image &) = delete;
    /// Converts Image to a bitmap
    virtual BitmapPtr asBitmap() const = 0;
    /// Converts Image to a texture
    virtual TexturePtr asTexture() const = 0;
    /// Returns the image's dimensions
    virtual Vector2i dimensions() const = 0;
    /// Returns the image's transparency mode
    constexpr TransparencyMode transparencyMode() const { return mode; }

protected:
    inline explicit Image(TransparencyMode mode) : mode(mode) { }

private:
    TransparencyMode mode;

};

}

#include "BitmapImage.h"
#include "TextureImage.h"
#include "PlacedImage.h"
