
#include "DesignEditorUIState.h"


bool DesignEditorUIState::LayerSelection::empty() const {
    return layerIDs.empty();
}

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

void DesignEditorUIState::LayerSelection::remove(const char *layerID) {
    if (layerID == nullptr || strlen(layerID) <= 0) {
        return;
    }

    const auto remIt = std::find_if(layerIDs.begin(), layerIDs.end(), [&layerID](const ODE_StringRef &idStr) {
        return strcmp(layerID, idStr.data) == 0;
    });
    if (remIt != layerIDs.end()) {
        layerIDs.erase(remIt);
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
