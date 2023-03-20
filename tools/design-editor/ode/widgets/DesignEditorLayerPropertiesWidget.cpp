
#include "DesignEditorLayerPropertiesWidget.h"

#include <imgui.h>

#include <ode-essentials.h>

using namespace ode;

namespace {

#define CHECK_IMEND(x) do { if ((x)) { ImGui::End(); return; } } while (false)

const ImU32 IM_COLOR_DARK_RED = 4278190233;

// TODO: Cleanup:
const char *blendModes[] = {
    "NORMAL",
    "PASS_THROUGH",
    "COLOR",
    "COLOR_BURN",
    "COLOR_DODGE",
    "DARKEN",
    "DARKER_COLOR",
    "DIFFERENCE",
    "DIVIDE",
    "EXCLUSION",
    "HARD_LIGHT",
    "HARD_MIX",
    "HUE",
    "LIGHTEN",
    "LIGHTER_COLOR",
    "LINEAR_BURN",
    "LINEAR_DODGE",
    "LINEAR_LIGHT",
    "LUMINOSITY",
    "MULTIPLY",
    "OVERLAY",
    "PIN_LIGHT",
    "SATURATION",
    "SCREEN",
    "SOFT_LIGHT",
    "SUBTRACT",
    "VIVID_LIGHT",
};

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

std::string layerTypeToString(ODE_LayerType layerType) {
    switch (layerType) {
        case ODE_LAYER_TYPE_UNSPECIFIED: return "-";
        case ODE_LAYER_TYPE_SHAPE: return "Shape";
        case ODE_LAYER_TYPE_TEXT: return "Text";
        case ODE_LAYER_TYPE_GROUP: return "Group";
        case ODE_LAYER_TYPE_MASK_GROUP: return "Mask Group";
        case ODE_LAYER_TYPE_COMPONENT_REFERENCE: return "Component Reference";
        case ODE_LAYER_TYPE_COMPONENT_INSTANCE: return "Component Instance";
    }
    return "-";
}

}

void drawLayerPropertiesWidget(const ODE_LayerList &layerList, DesignEditorContext &context) {
    ImGui::Begin("Selected Layer Properties");

    for (int i = 0; i < layerList.n; ++i) {
        const ODE_LayerList::Entry &layer = layerList.entries[i];
        if (context.selection.isSelected(layer.id.data)) {
            const auto layerPropName = [&layer](const char *invisibleId, const char *visibleLabel = "")->std::string {
                return std::string(visibleLabel)+std::string("##layer-")+std::string(invisibleId)+std::string("-")+std::string(layer.id.data);
            };

            bool layerVisible = true; // TODO: Get layer visibility
            float layerOpacity = 1.0f; // TODO: Get layer opacity
            const char *blendModeStr = "NORMAL"; // TODO: Get layer blend mode as string

            ODE_LayerMetrics layerMetrics;
            CHECK_IMEND(ode_component_getLayerMetrics(context.component, layer.id, &layerMetrics));

            const float a = static_cast<float>(layerMetrics.transformation.matrix[0]);
            const float b = static_cast<float>(layerMetrics.transformation.matrix[2]);
            const float c = static_cast<float>(layerMetrics.transformation.matrix[1]);
            const float d = static_cast<float>(layerMetrics.transformation.matrix[3]);
            const float trX = static_cast<float>(layerMetrics.transformation.matrix[4]);
            const float trY = static_cast<float>(layerMetrics.transformation.matrix[5]);

            Vector2f translation {
                trX,
                trY,
            };
            Vector2f scale {
                sqrt(a*a+b*b),
                sqrt(c*c+d*d),
            };
            float rotation = atan(c/d) * (180.0f/M_PI);

            const Vector2f origTranslation = translation;
            const Vector2f origScale = scale;
            const float origRotation = rotation;

            const std::string layerSectionHeader =
                std::string("[")+layerTypeToShortString(layer.type)+std::string("] ")+
                std::string(layer.id.data)+std::string(" ")+
                std::string("(")+std::string(layer.name.data)+std::string(")");

            if (ImGui::CollapsingHeader(layerSectionHeader.c_str())) {
                ImGui::Text("%s", "ID:");
                ImGui::SameLine(100);
                ImGui::Text("%s", layer.id.data);

                ImGui::Text("%s", "Name:");
                ImGui::SameLine(100);
                ImGui::Text("%s", layer.name.data);

                ImGui::Text("%s", "Type:");
                ImGui::SameLine(100);
                ImGui::Text("%s", layerTypeToString(layer.type).c_str());

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Visible:");
                ImGui::SameLine(100);
                if (ImGui::Checkbox(layerPropName("visibility").c_str(), &layerVisible)) {
                    // TODO: Update layer visiblity
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }

                ImGui::Text("Opacity:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat(layerPropName("opacity").c_str(), &layerOpacity)) {
                    // TODO: Update layer opacity
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }

                ImGui::Text("Bend mode:");
                ImGui::SameLine(100);
                if (ImGui::BeginCombo(layerPropName("blend-mode").c_str(), blendModeStr)) {
                    for (int n = 0; n < IM_ARRAYSIZE(blendModes); n++) {
                        bool isSelected = (blendModeStr == blendModes[n]);
                        if (ImGui::Selectable(blendModes[n], isSelected)) {
                            // TODO: Update layer blend mode
                            blendModeStr = blendModes[n];
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Translation:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat2((std::string("##translation-")+std::string(layer.id.data)).c_str(), &translation.x, 1.0f)) {
                    const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
                    CHECK_IMEND(ode_component_transformLayer(context.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }

                ImGui::Text("Scale:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat2(layerPropName("blend-scale").c_str(), &scale.x, 0.05f, 0.0f, 100.0f)) {
                    const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
                    CHECK_IMEND(ode_component_transformLayer(context.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }

                ImGui::Text("Rotation:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat(layerPropName("blend-rotation").c_str(), &rotation)) {
                    const float rotationChangeRad = -(rotation-origRotation)*M_PI/180.0f;
                    const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
                    CHECK_IMEND(ode_component_transformLayer(context.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Fill & stroke / text:");
                ImGui::Dummy(ImVec2(20.0f, 0.0f));
                ImGui::SameLine(50);
                ImGui::InputText(layerPropName("fill").c_str(), context.layerProperties.strokeFillText.data(), 50);

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Effects:");
                ImGui::SameLine(415);
                if (ImGui::SmallButton(layerPropName("effect-add", "+").c_str())) {
                    context.layerProperties.effects.emplace_back();
                }
                int effectToRemove = -1;
                for (size_t ei = 0; ei < context.layerProperties.effects.size(); ++ei) {
                    ImGui::Dummy(ImVec2(20.0f, 0.0f));
                    ImGui::SameLine(50);
                    ImGui::InputText(layerPropName((std::string("effect-")+std::to_string(ei)).c_str()).c_str(), context.layerProperties.effects[ei].data(), 50);
                    ImGui::SameLine(415);
                    if (ImGui::SmallButton((std::string("-##layer-effect-remove")+std::to_string(ei)).c_str())) {
                        effectToRemove = static_cast<int>(ei);
                    }
                }
                if (effectToRemove >= 0 && effectToRemove < static_cast<int>(context.layerProperties.effects.size())) {
                    context.layerProperties.effects.erase(context.layerProperties.effects.begin()+effectToRemove);
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_DARK_RED);
                ImGui::SameLine(100);
                if (ImGui::Button(layerPropName("delete", "Delete Layer [FUTURE_API]").c_str(), ImVec2 { 250, 20 })) {
                    // TODO: Remove layer when API available
                    // CHECK_IMEND(ode_component_removeLayer(context.component, selectedLayer.id));
                    CHECK_IMEND(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));
                }
                ImGui::PopStyleColor(1);

                ImGui::Dummy(ImVec2 { 0.0f, 20.0f });
            }
        }
    }

    ImGui::End();
}
