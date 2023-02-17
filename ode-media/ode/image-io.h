
#pragma once

#include "image-formats/png.h"
#include "image-formats/webp.h"
#include "image-formats/jpeg.h"
#include "image-formats/gif.h"
#include "image-formats/tiff.h"
#include "image-formats/rgba.h"

namespace ode {

/// Loads an image file at path into a bitmap object (supports PNG, JPEG, GIF, WEBP, TIFF)
Bitmap loadImage(const FilePath &path);

}
