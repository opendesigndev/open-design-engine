
#include "DesignEditorContext.h"

namespace {
bool isImGuiMultiselectKeyDown() {
    return (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
            ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) ||
            ImGui::IsKeyDown(ImGuiKey_RightShift) || ImGui::IsKeyDown(ImGuiKey_RightSuper));
}
}

void DesignEditorContext::Selection::select(const ODE_StringRef &layerID) {
    select(layerID.data);
}

void DesignEditorContext::Selection::select(const ODE_String &layerID) {
    select(layerID.data);
}

void DesignEditorContext::Selection::select(const char *layerID) {
    const int length = static_cast<int>(strlen(layerID));
    if (layerID == nullptr || length <= 0) {
        layerIDs.clear();
    } else {
        if (isImGuiMultiselectKeyDown()) {
            if (!isSelected(layerID)) {
                layerIDs.emplace_back(ODE_StringRef { layerID, length });
            }
        } else {
            layerIDs = { ODE_StringRef { layerID, length } };
        }
    }
}

bool DesignEditorContext::Selection::isSelected(const char *layerID) {
    for (const ODE_StringRef &id : layerIDs) {
        if (strcmp(id.data, layerID) == 0) {
            return true;
        }
    }
    return false;
}
