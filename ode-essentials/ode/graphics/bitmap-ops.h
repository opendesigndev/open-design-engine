
#pragma once

#include "Bitmap.h"

namespace ode {

void bitmapPremultiply(BitmapRef &bitmap);
void bitmapUnpremultiply(BitmapRef &bitmap);

void bitmapPremultiply(Bitmap &bitmap);
void bitmapUnpremultiply(Bitmap &bitmap);

}
