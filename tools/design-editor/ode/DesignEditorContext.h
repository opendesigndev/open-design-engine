
#pragma once

#include "DesignEditorDesign.h"

struct DesignEditorContext {
    ODE_EngineHandle engine;
    ODE_RendererContextHandle rc;
    ODE_PR1_FrameView frameView;
    /// The loaded / created design
    DesignEditorDesign design;

    int initialize();
    int destroy();
};
