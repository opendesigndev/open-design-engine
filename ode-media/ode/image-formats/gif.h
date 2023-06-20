
#pragma once

#ifdef ODE_MEDIA_GIF_SUPPORT
#ifndef __EMSCRIPTEN__

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectGifFormat(const byte *data, size_t length);

Bitmap loadGif(const FilePath &path);
Bitmap loadGif(FILE *file);
Bitmap loadGif(const byte *data, size_t length);

}

#endif
#endif
