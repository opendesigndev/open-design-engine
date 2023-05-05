
#include "DesignEditorToolbarWidget.h"

#include <imgui.h>

#include <ode-diagnostics.h>

#include "DesignEditorUIHelpers.h"
#include "DesignEditorUIValues.h"

namespace {

void addGroup(DesignEditorContext &context,
              DesignEditorComponent &component,
              const DesignEditorUIState::LayerSelection &layersSelection,
              const octopus::Octopus &newOctopusGroup) {
    std::string octopusLayerJson;
    octopus::Serializer::serialize(octopusLayerJson, *newOctopusGroup.content);

    const std::optional<std::string> insertionLayerIdOpt = findParentLayerId(component.layerList, layersSelection.layerIDs.front());
    if (!insertionLayerIdOpt.has_value()) {
        return;
    }

    for (const ODE_StringRef &layerId : layersSelection.layerIDs) {
        if (ode_component_removeLayer(component.component, layerId) != ODE_RESULT_OK) {
            return;
        }
    }

    ODE_ParseError parseError;
    if (ode_component_addLayer(component.component, ode_stringRef(*insertionLayerIdOpt), {}, ode_stringRef(octopusLayerJson), &parseError) != ODE_RESULT_OK) {
        return;
    }

    ode_component_listLayers(component.component, &component.layerList);
    ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
}

void drawGroupButtons(DesignEditorContext &context,
                      DesignEditorComponent &component,
                      const DesignEditorUIState::LayerSelection &layersSelection) {
    if (layersSelection.layerIDs.size() < 2) {
        return;
    }

    const std::optional<std::string> commonParentId = findParentLayerId(component.layerList, layersSelection.layerIDs.front());
    if (!commonParentId.has_value()) {
        return;
    }

    const bool allSelectedLayersSameParent = std::all_of(layersSelection.layerIDs.begin()+1, layersSelection.layerIDs.end(), [&layerList = component.layerList, &commonParentId](const ODE_StringRef &layerId)->bool {
        const std::optional<std::string> layerParentId = findParentLayerId(layerList, layerId);
        return layerParentId.has_value() && *commonParentId == * layerParentId;
    });
    if (!allSelectedLayersSameParent) {
        return;
    }

    ODE_String octopusString;
    ode_component_getOctopus(component.component, &octopusString);

    octopus::Octopus componentOctopus;
    octopus::Parser::parse(componentOctopus, octopusString.data);

    if (componentOctopus.content.has_value()) {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_LIGHT_BLUE);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COLOR_LIGHT_BLUE);

        const std::optional<octopus::Octopus> newOctopusGroup = [&layerIDs = layersSelection.layerIDs, &rootLayer = *componentOctopus.content, &layerList = component.layerList]()->std::optional<octopus::Octopus> {

            if (ImGui::Button("Group")) {
                ode::octopus_builder::GroupLayer group;
                for (const ODE_StringRef &layerId : layerIDs) {
                    const octopus::Layer *layer = findLayer(rootLayer, ode_stringDeref(layerId));
                    if (layer != nullptr) {
                        group.add(*layer);
                    }
                }
                const std::optional<std::string> idOpt = findAvailableLayerId("GROUP", layerList);
                if (idOpt.has_value()) {
                    group.id = *idOpt;
                    group.name = *idOpt;
                }
                return ode::octopus_builder::buildOctopus("Group", group);
            }

            ImGui::SameLine();

            if (ImGui::Button("Mask")) {
                const ODE_StringRef &maskLayerId = layerIDs.front();
                const octopus::Layer *maskLayer = findLayer(rootLayer, ode_stringDeref(maskLayerId));
                if (maskLayer != nullptr) {
                    ode::octopus_builder::MaskGroupLayer maskGroup(octopus::MaskBasis::BODY, *maskLayer);
                    for (size_t li = 1; li < layerIDs.size(); li++) {
                        const octopus::Layer *layer = findLayer(rootLayer, ode_stringDeref(layerIDs[li]));
                        if (layer != nullptr) {
                            maskGroup.add(*layer);
                        }
                    }
                    const std::optional<std::string> idOpt = findAvailableLayerId("MASK_GROUP", layerList);
                    if (idOpt.has_value()) {
                        maskGroup.id = *idOpt;
                        maskGroup.name = *idOpt;
                    }
                    return ode::octopus_builder::buildOctopus("Mask Group", maskGroup);
                }
            }

            return std::nullopt;
        } ();
        if (newOctopusGroup.has_value()) {
            addGroup(context, component, layersSelection, *newOctopusGroup);
        }

        ImGui::PopStyleColor(2);
    }
}

}

void drawToolbarWidget(DesignEditorContext &context,
                       DesignEditorComponent &component,
                       const DesignEditorUIState::LayerSelection &layersSelection,
                       DesignEditorUIState::Mode &mode) {
    ImGui::Begin("Toolbar");

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorUIState::Mode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorUIState::Mode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Select")) {
        mode = DesignEditorUIState::Mode::SELECT;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorUIState::Mode::MOVE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorUIState::Mode::MOVE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Move")) {
        mode = DesignEditorUIState::Mode::MOVE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorUIState::Mode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorUIState::Mode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Rectangle")) {
        mode = DesignEditorUIState::Mode::ADD_RECTANGLE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorUIState::Mode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorUIState::Mode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Ellipse")) {
        mode = DesignEditorUIState::Mode::ADD_ELLIPSE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorUIState::Mode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorUIState::Mode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Text")) {
        mode = DesignEditorUIState::Mode::ADD_TEXT;
    }
    ImGui::PopStyleColor(2);

    // Draw group and mask group buttons, handle the logic
    drawGroupButtons(context, component, layersSelection);

    ImGui::End();
}
