
#pragma once

#ifdef ODE_MEDIA_WEBP_SUPPORT
#ifndef __EMSCRIPTEN__

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectWebpFormat(const byte *data, size_t length);

Bitmap loadWebp(const FilePath &path);
Bitmap loadWebp(FILE *file);
Bitmap loadWebp(const byte *data, size_t length);

}

#endif
#endif
