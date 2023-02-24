
#pragma once

#include <ode-essentials.h>
#include "../image/Image.h"

namespace ode {

ImagePtr processAssetBitmap(const BitmapConstRef &bitmap);
ImagePtr processAssetBitmap(Bitmap &&bitmap);

}
