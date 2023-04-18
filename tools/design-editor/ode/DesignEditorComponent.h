
#pragma once

#include <ode-essentials.h>
#include <ode/api-base.h>
#include <ode/renderer-api.h>

/// Component data
struct DesignEditorComponent {
    DesignEditorComponent() = default;
    ~DesignEditorComponent();

    /// The file path of the Octopus file loaded.
    ode::FilePath filePath;
    /// Octopus Json string.
    std::string octopusJson;

    /// Component handle
    ODE_ComponentHandle component;
    /// Component metadata
    ODE_ComponentMetadata metadata = {};
    /// Bitmap renderered from the component
    ODE_Bitmap bitmap = {};
    /// Layer list
    ODE_LayerList layerList = {};
};
using DesignEditorComponents = std::vector<DesignEditorComponent>;
