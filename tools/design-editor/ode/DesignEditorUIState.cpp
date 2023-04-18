
#include "DesignEditorUIState.h"

void DesignEditorUIState::LayerSelection::select(const char *layerID) {
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

void DesignEditorUIState::LayerSelection::add(const char *layerID) {
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

void DesignEditorUIState::LayerSelection::clear() {
    layerIDs.clear();
}

bool DesignEditorUIState::LayerSelection::isSelected(const char *layerID) {
    for (const ODE_StringRef &id : layerIDs) {
        if (strcmp(id.data, layerID) == 0) {
            return true;
        }
    }
    return false;
}
