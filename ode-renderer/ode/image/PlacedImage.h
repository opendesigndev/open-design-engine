
#pragma once

#include <ode-logic.h>
#include "Image.h"

namespace ode {

/// Represents an abstract image pointer that has a set placement in the document, represented by bounds
class PlacedImagePtr : public ImagePtr {

public:
    inline PlacedImagePtr() { }
    inline PlacedImagePtr(std::nullptr_t) : ImagePtr(nullptr), placement() { }
    inline PlacedImagePtr(const ImagePtr &image, const ScaledBounds &bounds) : ImagePtr(image), placement(Vector2d(bounds.a), Vector2d(bounds.b)) { }
    inline PlacedImagePtr(const ImagePtr &image, const PixelBounds &bounds) : ImagePtr(image), placement(Vector2d(bounds.a), Vector2d(bounds.b)) { }
    inline PlacedImagePtr(ImagePtr &&image, const ScaledBounds &bounds) : ImagePtr((ImagePtr &&) image), placement(Vector2d(bounds.a), Vector2d(bounds.b)) { }
    inline PlacedImagePtr(ImagePtr &&image, const PixelBounds &bounds) : ImagePtr((ImagePtr &&) image), placement(Vector2d(bounds.a), Vector2d(bounds.b)) { }

    constexpr const ScaledBounds &bounds() const { return placement; }

private:
    ScaledBounds placement;

};


}
