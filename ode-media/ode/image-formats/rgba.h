
#pragma once

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectRgbaFormat(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
Bitmap loadRgba(const FilePath &path);
Bitmap loadRgba(FILE *file);
#endif
Bitmap loadRgba(const byte *data, size_t length);

#ifndef __EMSCRIPTEN__
bool saveRgba(const FilePath &path, SparseBitmapConstRef bitmap);
#endif

}
