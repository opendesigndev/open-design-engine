
#include "DesignEditorLoadedOctopus.h"

void DesignEditorLoadedOctopus::clear() {
    filePath = "";
    reloaded = true;
}

bool DesignEditorLoadedOctopus::isLoaded() const {
    return !filePath.empty() && !octopusJson.empty();
}
