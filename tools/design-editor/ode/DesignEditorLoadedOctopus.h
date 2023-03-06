
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

    void clear();
    bool isLoaded() const;
};
