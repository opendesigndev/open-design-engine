
#pragma once

#ifdef ODE_MEDIA_TIFF_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectTiffFormat(const byte *data, size_t length);

Bitmap loadTiff(const FilePath &path);
Bitmap loadTiff(FILE *file);

/// Stores the specified image bitmap as a TIFF file (supports floating-point bitmaps losslessly)
bool saveTiff(const FilePath &path, SparseBitmapConstRef bitmap);

}

#endif
