
#include "DesignEditorLayerPropertiesWidget.h"

#include <imgui.h>

#include <octopus/layer-change.h>
#include <ode-essentials.h>

using namespace ode;

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)
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

int changeLayerProperty(DesignEditorContext::Api &apiContext,
                        const ODE_StringRef &layerId,
                        const std::function<void(octopus::LayerChange::Values &)> changeFunction) {
    octopus::LayerChange layerChange;
    layerChange.subject = octopus::LayerChange::Subject::LAYER;
    layerChange.op = octopus::LayerChange::Op::PROPERTY_CHANGE;
    changeFunction(layerChange.values);

    std::string layerChangeStr;
    octopus::Serializer::serialize(layerChangeStr, layerChange);

    ODE_ParseError parseError { ODE_ParseError::Type::OK, 0 };
    CHECK(ode_component_modifyLayer(apiContext.component, layerId, ode_stringRef(layerChangeStr), &parseError));

    if (parseError.type == ODE_ParseError::OK) {
        CHECK(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));
        return 0;
    }

    return -1;
}

}

void drawLayerPropertiesWidget(const ODE_LayerList &layerList,
                               DesignEditorContext::Api &apiContext,
                               DesignEditorContext::LayerSelection &layerSelectionContext,
                               DesignEditorContext::LayerProperties &layerPropertiesContext) {
    ImGui::Begin("Selected Layer Properties");

    if (apiContext.component.ptr == nullptr) {
        ImGui::End();
        return;
    }

    ODE_String octopusString;
    ode_component_getOctopus(apiContext.component, &octopusString);

    octopus::Octopus componentOctopus;
    octopus::Parser::parse(componentOctopus, octopusString.data);

    if (!componentOctopus.content.has_value()) {
        ImGui::End();
        return;
    }

    for (int i = 0; i < layerList.n; ++i) {
        const ODE_LayerList::Entry &layer = layerList.entries[i];
        if (layerSelectionContext.isSelected(layer.id.data)) {
            const auto layerPropName = [&layer](const char *invisibleId, const char *visibleLabel = "")->std::string {
                return std::string(visibleLabel)+std::string("##layer-")+std::string(invisibleId)+std::string("-")+ode_stringDeref(layer.id);
            };

            const octopus::Layer *octopusLayer = findLayer(*componentOctopus.content, ode_stringDeref(layer.id));
            if (octopusLayer == nullptr) {
                continue;
            }

            bool layerVisible = octopusLayer->visible;
            float layerOpacity = octopusLayer->opacity;
            const octopus::BlendMode blendMode = octopusLayer->blendMode;
            const int blendModeI = static_cast<int>(blendMode);

            ODE_LayerMetrics layerMetrics;
            CHECK_IMEND(ode_component_getLayerMetrics(apiContext.component, layer.id, &layerMetrics));

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
                ode_stringDeref(layer.id)+std::string(" ")+
                std::string("(")+ode_stringDeref(layer.name)+std::string(")");

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
                    CHECK_IMEND(changeLayerProperty(apiContext, layer.id, [layerVisible](octopus::LayerChange::Values &values) {
                        values.visible = layerVisible;
                    }));
                }

                ImGui::Text("Opacity:");
                ImGui::SameLine(100);
                if (ImGui::SliderFloat(layerPropName("opacity").c_str(), &layerOpacity, 0.0f, 1.0f)) {
                    CHECK_IMEND(changeLayerProperty(apiContext, layer.id, [layerOpacity](octopus::LayerChange::Values &values) {
                        values.opacity = layerOpacity;
                    }));
                }

                ImGui::Text("Bend mode:");
                ImGui::SameLine(100);
                if (ImGui::BeginCombo(layerPropName("blend-mode").c_str(), blendModes[blendModeI])) {
                    for (int bmI = 0; bmI < IM_ARRAYSIZE(blendModes); bmI++) {
                        const bool isSelected = (blendModeI == bmI);
                        if (ImGui::Selectable(blendModes[bmI], isSelected)) {
                            CHECK_IMEND(changeLayerProperty(apiContext, layer.id, [bmI](octopus::LayerChange::Values &values) {
                                values.blendMode = static_cast<octopus::BlendMode>(bmI);
                            }));
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
                if (ImGui::DragFloat2((std::string("##translation-")+ode_stringDeref(layer.id)).c_str(), &translation.x, 1.0f)) {
                    const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
                    CHECK_IMEND(ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));
                }

                ImGui::Text("Scale:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat2(layerPropName("blend-scale").c_str(), &scale.x, 0.05f, 0.0f, 100.0f)) {
                    const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
                    CHECK_IMEND(ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));
                }

                ImGui::Text("Rotation:");
                ImGui::SameLine(100);
                if (ImGui::DragFloat(layerPropName("blend-rotation").c_str(), &rotation)) {
                    const float rotationChangeRad = -(rotation-origRotation)*M_PI/180.0f;
                    const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
                    CHECK_IMEND(ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
                    CHECK_IMEND(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Fill & stroke / text:");
                ImGui::Dummy(ImVec2(20.0f, 0.0f));
                ImGui::SameLine(50);
                ImGui::InputText(layerPropName("fill").c_str(), layerPropertiesContext.strokeFillText.data(), 50);

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::Text("Effects:");
                ImGui::SameLine(415);
                if (ImGui::SmallButton(layerPropName("effect-add", "+").c_str())) {
                    layerPropertiesContext.effects.emplace_back();
                }
                int effectToRemove = -1;
                for (size_t ei = 0; ei < layerPropertiesContext.effects.size(); ++ei) {
                    ImGui::Dummy(ImVec2(20.0f, 0.0f));
                    ImGui::SameLine(50);
                    ImGui::InputText(layerPropName((std::string("effect-")+std::to_string(ei)).c_str()).c_str(), layerPropertiesContext.effects[ei].data(), 50);
                    ImGui::SameLine(415);
                    if (ImGui::SmallButton((std::string("-##layer-effect-remove")+std::to_string(ei)).c_str())) {
                        effectToRemove = static_cast<int>(ei);
                    }
                }
                if (effectToRemove >= 0 && effectToRemove < static_cast<int>(layerPropertiesContext.effects.size())) {
                    layerPropertiesContext.effects.erase(layerPropertiesContext.effects.begin()+effectToRemove);
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_DARK_RED);
                ImGui::SameLine(100);
                if (ImGui::Button(layerPropName("delete", "Delete Layer [FUTURE_API]").c_str(), ImVec2 { 250, 20 })) {
                    // TODO: Remove layer when API available
                    // CHECK_IMEND(ode_component_removeLayer(context.component, selectedLayer.id));
                    CHECK_IMEND(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));
                }
                ImGui::PopStyleColor(1);

                ImGui::Dummy(ImVec2 { 0.0f, 20.0f });
            }
        }
    }

    ImGui::End();
}
