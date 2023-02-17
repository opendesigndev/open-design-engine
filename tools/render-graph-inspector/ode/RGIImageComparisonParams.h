
#pragma once

struct RGIImageComparisonParams {
    int selectedComparisonType = 1;
    float sliderXPos = 0.5f;
    float sliderStrokeWidth = 0.005f;
    float diffWeight = 1.0f;

    void moveSlider(float d);
    void moveDiffWeight(float d);
};
inline bool operator==(const RGIImageComparisonParams &a, const RGIImageComparisonParams &b) {
    return a.selectedComparisonType == b.selectedComparisonType && a.sliderXPos == b.sliderXPos && a.sliderStrokeWidth == b.sliderStrokeWidth && a.diffWeight == b.diffWeight;
}
inline bool operator!=(const RGIImageComparisonParams &a, const RGIImageComparisonParams &b) {
    return !(a == b);
}
// Comparison types titles
constexpr const char* rgiImageComparisonTypes[4] = { "Diff", "Slider", "Fade", "Side by side" };
constexpr const int rgiImageComparisonTypesCount = sizeof(rgiImageComparisonTypes)/sizeof(rgiImageComparisonTypes[0]);
