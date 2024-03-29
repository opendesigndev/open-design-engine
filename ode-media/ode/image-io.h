
#pragma once

#include "image-formats/png.h"
#include "image-formats/jpeg.h"
#include "image-formats/rgba.h"
#include "image-formats/gif.h"
#include "image-formats/tiff.h"
#include "image-formats/webp.h"

namespace ode {

/// Loads an image file at path into a bitmap object (supports PNG, JPEG, GIF, WEBP, TIFF)
Bitmap loadImage(const FilePath &path);

/// Loads an image stored in memory into a bitmap object (supports JPEG)
Bitmap loadImage(const byte *data, size_t length);

}
