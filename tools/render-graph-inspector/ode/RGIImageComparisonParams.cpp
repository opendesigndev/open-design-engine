
#include "RGIImageComparisonParams.h"

#include <algorithm>

void RGIImageComparisonParams::moveSlider(float d) {
    sliderXPos = std::clamp(sliderXPos + d, 0.0f, 1.0f);
}

void RGIImageComparisonParams::moveDiffWeight(float d) {
    diffWeight = std::clamp(diffWeight + d, 0.0f, 100.0f);
}
