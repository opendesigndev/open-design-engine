
#pragma once

struct RGIImageVisualizationParams {
    int selectedDisplayMode = 2;
};
inline bool operator==(const RGIImageVisualizationParams &a, const RGIImageVisualizationParams &b) {
    return a.selectedDisplayMode == b.selectedDisplayMode;
}
inline bool operator!=(const RGIImageVisualizationParams &a, const RGIImageVisualizationParams &b) {
    return !(a == b);
}
// Display modes titles
constexpr const char* rgiImageDisplayModes[4] = { "Transparent", "Black bcg", "White bcg", "Ignore alpha" };
