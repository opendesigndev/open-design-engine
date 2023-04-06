
#include "DesignEditorLayerListWidget.h"

#include <vector>

#include <imgui.h>

#include <ode/logic-api.h>

#include "DesignEditorUIHelpers.h"
#include "DesignEditorUIValues.h"

namespace {

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

    const std::string layerLabel =
        std::string("[")+layerTypeToShortString(rootLayer.type)+std::string("] ")+
        ode_stringDeref(rootLayer.id)+std::string(" ")+
        std::string("(")+ode_stringDeref(rootLayer.name)+std::string(")");

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
            if (isImGuiMultiselectKeyModifierPressed()) {
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
