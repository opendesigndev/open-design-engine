
#pragma once

#ifdef ODE_MEDIA_GIF_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectGifFormat(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
Bitmap loadGif(const FilePath &path);
Bitmap loadGif(FILE *file);
#endif
Bitmap loadGif(const byte *data, size_t length);

}

#endif
