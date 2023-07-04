
#pragma once

#ifdef ODE_MEDIA_PNG_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectPngFormat(const byte *data, size_t length);

Bitmap loadPng(const FilePath &path);
Bitmap loadPng(FILE *file);
Bitmap loadPng(const byte *data, size_t length);

/// Stores the specified image bitmap as a PNG file
bool savePng(const FilePath &path, SparseBitmapConstRef bitmap);

/// Encode bitmap data as PNG and write into memory buffer
bool writePng(SparseBitmapConstRef bitmap, std::vector<byte> &pngData);

}

#endif
