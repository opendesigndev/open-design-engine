
#pragma once

#ifdef ODE_MEDIA_TIFF_SUPPORT
#ifndef __EMSCRIPTEN__

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectTiffFormat(const byte *data, size_t length);

Bitmap loadTiff(const FilePath &path);
Bitmap loadTiff(FILE *file);
Bitmap loadTiff(const byte *data, size_t length);

/// Stores the specified image bitmap as a TIFF file (supports floating-point bitmaps losslessly)
bool saveTiff(const FilePath &path, SparseBitmapConstRef bitmap);

}

#endif
#endif
