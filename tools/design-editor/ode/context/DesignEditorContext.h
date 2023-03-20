
#pragma once

#include <imgui.h>

#include <ode/api-base.h>
#include <ode/renderer-api.h>
#include <ode-renderer.h>

enum class DesignEditorMode {
    SELECT,
    ADD_RECTANGLE,
    ADD_ELLIPSE,
    ADD_TEXT,
};

struct DesignEditorContext {
    ODE_EngineHandle engine;
    ODE_EngineAttributes engineAttribs;
    ODE_DesignHandle design;
    ODE_ComponentMetadata metadata = { };
    ODE_RendererContextHandle rc;
    ODE_DesignImageBaseHandle imageBase;
    ODE_ComponentHandle component;
    ODE_Bitmap bitmap;
    ODE_PR1_FrameView frameView;

    struct FileDialog {
        std::string filePath = ".";
        std::string fileName;
    } fileDialog;

    struct Canvas {
        bool isMouseOver = false;
        ImVec2 bbSize;
        ImVec2 bbMin;
        ImVec2 bbMax;
        float zoom = 1.0f;
    } canvas;

    struct Textures {
        ode::TexturePtr designImageTexture = nullptr;
    } textures;

    struct Selection {
        std::vector<ODE_StringRef> layerIDs {};

        void select(const ODE_StringRef &layerID);
        void select(const ODE_String &layerID);
        void select(const char *layerID);
        bool isSelected(const char *layerID);
    } selection;

    // TODO: Remove and read from the selected layers
    struct LayerProperties {
        std::string strokeFillText;
        std::vector<std::string> effects;
    } layerProperties;

};
