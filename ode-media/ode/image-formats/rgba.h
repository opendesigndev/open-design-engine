
#pragma once

#include <cstdio>
#include <ode-essentials.h>

namespace ode {

bool detectRgbaFormat(const byte *data, size_t length);

Bitmap loadRgba(const FilePath &path);
Bitmap loadRgba(FILE *file);
Bitmap loadRgba(const byte *data, size_t length);

bool saveRgba(const FilePath &path, SparseBitmapConstRef bitmap);

}
