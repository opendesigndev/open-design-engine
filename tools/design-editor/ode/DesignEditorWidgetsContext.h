
#pragma once

struct DesignEditorWidgetsContext {
    bool showToolbar = true;
    bool showLayerList = true;
    bool showDesignView = true;
    bool showLayerProperties = true;
    bool showImGuiDebugger = false;
};

inline bool operator==(const DesignEditorWidgetsContext &a, const DesignEditorWidgetsContext &b) {
    return
        a.showToolbar == b.showToolbar &&
        a.showLayerList == b.showLayerList &&
        a.showDesignView == b.showDesignView &&
        a.showLayerProperties == b.showLayerProperties &&
        a.showImGuiDebugger == b.showImGuiDebugger;
}
inline bool operator!=(const DesignEditorWidgetsContext &a, const DesignEditorWidgetsContext &b) {
    return !(a == b);
}
