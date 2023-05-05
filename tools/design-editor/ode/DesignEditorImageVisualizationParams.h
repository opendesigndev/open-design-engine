
#pragma once

struct DesignEditorImageVisualizationParams {
    int selectedDisplayMode = 2;
};
inline bool operator==(const DesignEditorImageVisualizationParams &a, const DesignEditorImageVisualizationParams &b) {
    return a.selectedDisplayMode == b.selectedDisplayMode;
}
inline bool operator!=(const DesignEditorImageVisualizationParams &a, const DesignEditorImageVisualizationParams &b) {
    return !(a == b);
}
// Display modes titles
constexpr const char* designEditorImageDisplayModes[4] = { "Transparent", "Black bcg", "White bcg", "Ignore alpha" };
