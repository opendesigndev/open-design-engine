
#include "DesignEditorLoadedOctopus.h"

void DesignEditorLoadedOctopus::clear() {
    filePath = "";
}

bool DesignEditorLoadedOctopus::isLoaded() const {
    return !filePath.empty() && !octopusJson.empty();
}
