
#pragma once

#ifdef ODE_MEDIA_PNG_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectPngFormat(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
Bitmap loadPng(const FilePath &path);
Bitmap loadPng(FILE *file);
#endif
Bitmap loadPng(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
/// Stores the specified image bitmap as a PNG file
bool savePng(const FilePath &path, SparseBitmapConstRef bitmap);
#endif

/// Encode bitmap data as PNG and write into memory buffer
bool writePng(SparseBitmapConstRef bitmap, std::vector<byte> &pngData);

}

#endif
