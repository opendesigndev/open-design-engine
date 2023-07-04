
#pragma once

#include "DesignEditorComponent.h"

/// A representation of a single design
struct DesignEditorDesign {
    bool empty() const;

    int create(ODE_EngineHandle engine, ODE_RendererContextHandle rc);
    int destroy();

    /// A single design
    ODE_DesignHandle design;
    /// A single image base for the design
    ODE_DesignImageBaseHandle imageBase;
    /// Images directory
    ode::FilePath imageDirectory;
    /// The loaded/created components
    DesignEditorComponents components;
};
