
#pragma once

#ifdef ODE_MEDIA_WEBP_SUPPORT

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectWebpFormat(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
Bitmap loadWebp(const FilePath &path);
Bitmap loadWebp(FILE *file);
#endif
Bitmap loadWebp(const byte *data, size_t length);

}

#endif
