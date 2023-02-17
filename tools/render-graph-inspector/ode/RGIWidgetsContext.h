
#pragma once

struct RGIWidgetsContext {
    bool showResultImage = true;
    bool showRenderGraph = true;
    bool showRenderGraphInfo = false;
    bool showSelectedImages = false;
    bool showImageComparison = false;
    bool showImGuiDebugger = false;
    bool showVersionsComparison = true;
};

inline bool operator==(const RGIWidgetsContext &a, const RGIWidgetsContext &b) {
    return
        a.showResultImage == b.showResultImage &&
        a.showRenderGraph == b.showRenderGraph &&
        a.showRenderGraphInfo == b.showRenderGraphInfo &&
        a.showSelectedImages == b.showSelectedImages &&
        a.showImageComparison == b.showImageComparison &&
        a.showImGuiDebugger == b.showImGuiDebugger &&
        a.showVersionsComparison == b.showVersionsComparison;
}
inline bool operator!=(const RGIWidgetsContext &a, const RGIWidgetsContext &b) {
    return !(a == b);
}
