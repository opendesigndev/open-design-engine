
#include "DesignEditorToolbarWidget.h"

#include <imgui.h>

#include <ode-diagnostics.h>

namespace {
const ImU32 IM_COLOR_DARK_RED = 4278190233;
const ImU32 IM_COLOR_LIGHT_BLUE = 4294941081;

const octopus::Layer *findLayer(const octopus::Layer &layer, const std::string &layerId) {
    if (layer.id == layerId) {
        return &layer;
    }
    if (layer.mask.has_value()) {
        const octopus::Layer *l = findLayer(*layer.mask, layerId);
        if (l != nullptr) {
            return l;
        }
    }
    if (layer.layers.has_value()) {
        for (const octopus::Layer &childLayer : *layer.layers) {
            const octopus::Layer *l = findLayer(childLayer, layerId);
            if (l != nullptr) {
                return l;
            }
        }
    }
    return nullptr;
}

std::optional<std::string> findParentLayerId(const ODE_LayerList &layerList, const ODE_StringRef &layerId) {
    for (int i = 0; i < layerList.n; ++i) {
        const ODE_LayerList::Entry &layer = layerList.entries[i];
        if (strcmp(layerId.data, layer.id.data) == 0) {
            return ode_stringDeref(layer.parentId);
        }
    }
    return std::nullopt;
}

void addGroup(DesignEditorContext::Api &apiContext,
              ODE_LayerList &layerList,
              const DesignEditorContext::LayerSelection &layersSelectionContext,
              const octopus::Octopus &newOctopusGroup) {
    std::string octopusLayerJson;
    octopus::Serializer::serialize(octopusLayerJson, *newOctopusGroup.content);

    const std::optional<std::string> insertionLayerIdOpt = findParentLayerId(layerList, layersSelectionContext.layerIDs.front());
    if (!insertionLayerIdOpt.has_value()) {
        return;
    }

    ODE_ParseError parseError;
    const ODE_Result result = ode_component_addLayer(apiContext.component, ode_stringRef(*insertionLayerIdOpt), {}, ode_stringRef(octopusLayerJson), &parseError);

    if (result != ODE_RESULT_OK) {
        return;
    }

    ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
    ode_component_listLayers(apiContext.component, &layerList);

    for (const ODE_StringRef &layerId : layersSelectionContext.layerIDs) {
        // TODO: Remove original grouped layers when added support for removal
        // ode_component_removeLayer(apiContext.component, layerId);
    }
}

void drawGroupButtons(DesignEditorContext::Api &apiContext,
                      ODE_LayerList &layerList,
                      const DesignEditorContext::LayerSelection &layersSelectionContext) {
    if (layersSelectionContext.layerIDs.size() < 2) {
        return;
    }

    const std::optional<std::string> commonParentId = findParentLayerId(layerList, layersSelectionContext.layerIDs.front());
    if (!commonParentId.has_value()) {
        return;
    }

    const bool allSelectedLayersSameParent = std::all_of(layersSelectionContext.layerIDs.begin()+1, layersSelectionContext.layerIDs.end(), [&layerList, &commonParentId](const ODE_StringRef &layerId)->bool {
        const std::optional<std::string> layerParentId = findParentLayerId(layerList, layerId);
        return layerParentId.has_value() && *commonParentId == * layerParentId;
    });
    if (!allSelectedLayersSameParent) {
        return;
    }

    ODE_String octopusString;
    ode_component_getOctopus(apiContext.component, &octopusString);

    octopus::Octopus componentOctopus;
    octopus::Parser::parse(componentOctopus, octopusString.data);

    if (componentOctopus.content.has_value()) {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_LIGHT_BLUE);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COLOR_LIGHT_BLUE);

        const std::optional<octopus::Octopus> newOctopusGroup = [&layerIDs = layersSelectionContext.layerIDs, &rootLayer = *componentOctopus.content]()->std::optional<octopus::Octopus> {

            if (ImGui::Button("Group")) {
                ode::octopus_builder::GroupLayer group;
                for (const ODE_StringRef &layerId : layerIDs) {
                    const octopus::Layer *layer = findLayer(rootLayer, ode_stringDeref(layerId));
                    if (layer != nullptr) {
                        group.add(*layer);
                    }
                }
                return ode::octopus_builder::buildOctopus("Group (WIP)", group);
            }

            ImGui::SameLine();

            if (ImGui::Button("Mask")) {
                const ODE_StringRef &maskLayerId = layerIDs.front();
                const octopus::Layer *maskLayer = findLayer(rootLayer, ode_stringDeref(maskLayerId));
                if (maskLayer != nullptr) {
                    ode::octopus_builder::MaskGroupLayer maskGroup(octopus::MaskBasis::SOLID, *maskLayer);
                    for (size_t li = 1; li < layerIDs.size(); li++) {
                        const octopus::Layer *layer = findLayer(rootLayer, ode_stringDeref(layerIDs[li]));
                        if (layer != nullptr) {
                            maskGroup.add(*layer);
                        }
                    }
                    return ode::octopus_builder::buildOctopus("Mask Group (WIP)", maskGroup);
                }
            }

            return std::nullopt;
        } ();
        if (newOctopusGroup.has_value()) {
            addGroup(apiContext, layerList, layersSelectionContext, *newOctopusGroup);
        }

        ImGui::PopStyleColor(2);
    }
}

}

void drawToolbarWidget(DesignEditorContext::Api &apiContext,
                       ODE_LayerList &layerList,
                       const DesignEditorContext::LayerSelection &layersSelectionContext,
                       DesignEditorMode &mode) {
    ImGui::Begin("Toolbar");

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Select")) {
        mode = DesignEditorMode::SELECT;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Rectangle")) {
        mode = DesignEditorMode::ADD_RECTANGLE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Ellipse")) {
        mode = DesignEditorMode::ADD_ELLIPSE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Text")) {
        mode = DesignEditorMode::ADD_TEXT;
    }
    ImGui::PopStyleColor(2);

    // Draw group and mask group buttons, handle the logic
    drawGroupButtons(apiContext, layerList, layersSelectionContext);

    ImGui::End();
}
