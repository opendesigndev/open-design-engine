
#pragma once

#include <ode-essentials.h>
#include <ode/api-base.h>
#include <ode/renderer-api.h>

/// Component data
struct DesignEditorComponent {
    DesignEditorComponent() = default;
    ~DesignEditorComponent();

    /// Json representation of the component
    std::string octopusJson;

    /// Component handle
    ODE_ComponentHandle component;
    /// Metadata
    ODE_ComponentMetadata metadata = {};
    /// Bitmap renderered from the component
    ODE_Bitmap bitmap = {};
    /// List of layers in the component
    ODE_LayerList layerList = {};
};
using DesignEditorComponents = std::vector<DesignEditorComponent>;
