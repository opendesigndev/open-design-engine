
#include "DesignEditorLayerListWidget.h"

#include <vector>

#include <imgui.h>

#include <ode/logic-api.h>

namespace {

const ImU32 IM_COLOR_WHITE = 4294967295;
const ImU32 IM_COLOR_LIGHT_BLUE = 4294941081;

std::string layerTypeToShortString(ODE_LayerType layerType) {
    switch (layerType) {
        case ODE_LAYER_TYPE_UNSPECIFIED: return "-";
        case ODE_LAYER_TYPE_SHAPE: return "S";
        case ODE_LAYER_TYPE_TEXT: return "T";
        case ODE_LAYER_TYPE_GROUP: return "G";
        case ODE_LAYER_TYPE_MASK_GROUP: return "M";
        case ODE_LAYER_TYPE_COMPONENT_REFERENCE: return "CR";
        case ODE_LAYER_TYPE_COMPONENT_INSTANCE: return "CI";
    }
    return "-";
}

void drawLayerListRecursiveStep(const ODE_LayerList &layerList,
                                int idx,
                                int &idxClicked,
                                const std::vector<ODE_StringRef> &selectedLayerIDs) {
    if (idx >= layerList.n) {
        return;
    }

    const auto areEq = [](const ODE_StringRef &a, const ODE_StringRef &b)->bool {
        return a.length == b.length && strcmp(a.data, b.data) == 0;
    };

    const ODE_LayerList::Entry &rootLayer = layerList.entries[idx];
    const bool hasAnyChildren = (idx+1 < layerList.n) && areEq(layerList.entries[idx+1].parentId, rootLayer.id);
    const std::string layerLabel = "["+layerTypeToShortString(rootLayer.type)+"] "+std::string(rootLayer.id.data);

    const bool isSelected = std::find_if(selectedLayerIDs.begin(), selectedLayerIDs.end(), [&id = rootLayer.id](const ODE_StringRef &selectedLayerID)->bool {
        return strcmp(id.data, selectedLayerID.data) == 0;
    }) != selectedLayerIDs.end();
    const ImU32 listEntryColor = isSelected ? IM_COLOR_LIGHT_BLUE : IM_COLOR_WHITE;

    if (hasAnyChildren) {
        ImGui::PushStyleColor(ImGuiCol_Text, listEntryColor);
        const bool isOpened = ImGui::TreeNodeEx((layerLabel+std::string("##")+std::string(rootLayer.id.data)).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);
        ImGui::PopStyleColor(1);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            idxClicked = idx;
        }
        if (isOpened) {
            for (int i = idx+1; i < layerList.n; i++) {
                const ODE_LayerList::Entry &entry = layerList.entries[i];
                if (areEq(entry.parentId, rootLayer.id)) {
                    drawLayerListRecursiveStep(layerList, i, idxClicked, selectedLayerIDs);
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, listEntryColor);
        ImGui::BulletText("%s", layerLabel.c_str());
        ImGui::PopStyleColor(1);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            idxClicked = idx;
        }
    }
}

}

void drawLayerListWidget(const DesignEditorLoadedOctopus &loadedOctopus,
                         DesignEditorContext::LayerSelection &layerSelectionContext) {
    ImGui::Begin("Layer List");

    if (loadedOctopus.isLoaded()) {
        int idxClicked = -1;
        drawLayerListRecursiveStep(loadedOctopus.layerList, 0, idxClicked, layerSelectionContext.layerIDs);
        if (idxClicked >= 0) {
            const bool isMultiselect =
                ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) ||
                ImGui::IsKeyDown(ImGuiKey_RightShift) || ImGui::IsKeyDown(ImGuiKey_RightSuper);

            if (isMultiselect) {
                layerSelectionContext.add(loadedOctopus.layerList.entries[idxClicked].id.data);
            } else {
                layerSelectionContext.select(loadedOctopus.layerList.entries[idxClicked].id.data);
            }
        }
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}
