
#pragma once

#ifdef ODE_MEDIA_JPEG_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectJpegFormat(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
Bitmap loadJpeg(const FilePath &path);
Bitmap loadJpeg(FILE *file);
#endif
Bitmap loadJpeg(const byte *data, size_t length);

}

#endif
