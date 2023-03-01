
#pragma once

struct DesignEditorWidgetsContext {
    bool showResultImage = true;
    bool showImGuiDebugger = false;
};

inline bool operator==(const DesignEditorWidgetsContext &a, const DesignEditorWidgetsContext &b) {
    return
        a.showResultImage == b.showResultImage &&
        a.showImGuiDebugger == b.showImGuiDebugger;
}
inline bool operator!=(const DesignEditorWidgetsContext &a, const DesignEditorWidgetsContext &b) {
    return !(a == b);
}
