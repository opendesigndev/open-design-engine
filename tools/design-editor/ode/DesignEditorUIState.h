
#pragma once

#include <string>
#include <optional>

#include <imgui.h>

#include <ode-renderer.h>
#include <ode/api-base.h>

#include "DesignEditorImageVisualizationParams.h"

struct DesignEditorUIState {
    /// Editor mode
    enum class Mode {
        SELECT,
        ADD_RECTANGLE,
        ADD_ELLIPSE,
        ADD_TEXT,
    } mode = Mode::SELECT;

    /// ImGui window global state
    struct ImGuiWindow {
        const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } imGuiWindow;

    /// Current and previous state of widgets displayed
    struct Widgets {
        bool showToolbar = true;
        bool showLayerList = true;
        bool showDesignView = true;
        bool showLayerProperties = true;
        bool showImGuiDebugger = false;
    } widgets;

    /// State of the file dialog
    struct FileDialog {
        std::string octopusFilePath = ".";
        std::string octopusFileName;
        std::string imageFilePath = ".";
        std::string imageFileName;
    } fileDialog;

    /// Textures used by the design editor
    struct Textures {
        ode::TexturePtr designImageTexture = nullptr;
    } textures;

    /// Canvas state
    struct Canvas {
        bool isMouseOver = false;
        ImVec2 bbSize;
        ImVec2 bbMin;
        ImVec2 bbMax;
        float zoom = 1.0f;
        std::optional<ImVec2> mouseClickPos;
        std::optional<ImVec2> mouseDragPos;
    } canvas;

    /// Layer selection
    struct LayerSelection {
        std::vector<ODE_StringRef> layerIDs {};

        void select(const char *layerID);
        void add(const char *layerID);
        void remove(const char *layerID);
        void clear();

        bool isSelected(const char *layerID);
    } layerSelection;

    /// Current image visualization params
    DesignEditorImageVisualizationParams imageVisualizationParams;
};

inline bool operator==(const DesignEditorUIState::Widgets &a, const DesignEditorUIState::Widgets &b) {
    return
        a.showToolbar == b.showToolbar &&
        a.showLayerList == b.showLayerList &&
        a.showDesignView == b.showDesignView &&
        a.showLayerProperties == b.showLayerProperties &&
        a.showImGuiDebugger == b.showImGuiDebugger;
}
inline bool operator!=(const DesignEditorUIState::Widgets &a, const DesignEditorUIState::Widgets &b) {
    return !(a == b);
}
