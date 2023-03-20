
#pragma once

#include <optional>

#include <ode/renderer-api.h>
#include <ode-essentials.h>

using namespace ode;

struct DesignEditorLoadedOctopus {
    /// The file path of the Octopus file loaded.
    FilePath filePath;
    /// Octopus Json string.
    std::string octopusJson;

    ODE_LayerList layerList;

    /// A flag that is true just after a new Octopus file is loaded
    bool reloaded = false;

    void clear();
    bool isLoaded() const;
};
