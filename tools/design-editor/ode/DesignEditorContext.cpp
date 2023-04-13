
#include "DesignEditorContext.h"

void DesignEditorContext::LoadedOctopus::clear() {
    filePath = "";
}

bool DesignEditorContext::LoadedOctopus::isLoaded() const {
    return !filePath.empty() && !octopusJson.empty();
}

void DesignEditorContext::LayerSelection::select(const char *layerID) {
    clear();

    if (layerID == nullptr) {
        return;
    }

    const int length = static_cast<int>(strlen(layerID));
    if (length <= 0) {
        return;
    }

    layerIDs = { ODE_StringRef { layerID, length } };
}

void DesignEditorContext::LayerSelection::add(const char *layerID) {
    if (layerID == nullptr) {
        return;
    }

    const int length = static_cast<int>(strlen(layerID));
    if (length <= 0) {
        return;
    }

    if (!isSelected(layerID)) {
        layerIDs.emplace_back(ODE_StringRef { layerID, length });
    }
}

void DesignEditorContext::LayerSelection::clear() {
    layerIDs.clear();
}

bool DesignEditorContext::LayerSelection::isSelected(const char *layerID) {
    for (const ODE_StringRef &id : layerIDs) {
        if (strcmp(id.data, layerID) == 0) {
            return true;
        }
    }
    return false;
}
