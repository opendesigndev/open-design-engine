
#include "DesignEditorLayerPropertiesWidget.h"

#include <imgui.h>

#include <octopus/layer-change.h>
#include <ode-essentials.h>

using namespace ode;

namespace {

const ImU32 IM_COLOR_DARK_RED = 4278190233;

const char *BLEND_MODES_STR[] = {
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

const char *STROKE_POSITIONS_STR[] = {
    "OUTSIDE",
    "CENTER",
    "INSIDE",
};

const char *STROKE_STYLES_STR[] = {
    "SOLID",
    "DASHED",
    "DOTTED",
};

const char *FILL_TYPES_STR[] = {
    "COLOR",
    "GRADIENT",
    "IMAGE",
};

const char *FILL_GRADIENT_TYPES_STR[] = {
    "LINEAR",
    "RADIAL",
    "ANGULAR",
    "DIAMOND",
};

const char *FILL_GRADIENT_INTERPOLATIONS_STR[] = {
    "LINEAR",
    "POWER",
    "REVERSE_POWER",
};

const char *FILL_POSITIONING_LAYOUTS_STR[] = {
    "STRETCH",
    "FILL",
    "FIT",
    "TILE"
};

const char *FILL_POSITIONING_ORIGINS_STR[] = {
    "LAYER",
    "PARENT",
    "COMPONENT",
    "ARTBOARD"
};

const char *IMAGE_REF_TYPES_STR[] = {
    "PATH",
    "RESOURCE_REF",
};

const char *EFFECT_TYPES_STR[] = {
    "OVERLAY",
    "STROKE",
    "DROP_SHADOW",
    "INNER_SHADOW",
    "OUTER_GLOW",
    "INNER_GLOW",
    "GAUSSIAN_BLUR",
    "BOUNDED_BLUR",
    "BLUR",
    "OTHER"
};

const std::map<int, const char*> EFFECT_BASIS_MAP {
    { 1, "BODY" },
    { 3, "BODY_AND_STROKES" },
    { 4, "FILL" },
    { 7, "LAYER_AND_EFFECTS" },
    { 8, "BACKGROUND" },
};

const octopus::Fill DEFAULT_FILL {
    octopus::Fill::Type::COLOR,
    true,
    octopus::BlendMode::NORMAL,
    octopus::Color{1.0f,1.0f,1.0f,1.0f},
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt
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

using LayerChangeFunction = std::function<void(octopus::LayerChange::Values &)>;
const LayerChangeFunction NO_LAYER_CHANGE = [](octopus::LayerChange::Values &) {};

int applyLayerChange(octopus::LayerChange::Subject subject,
                     octopus::LayerChange::Op operation,
                     DesignEditorContext::Api &apiContext,
                     const ODE_StringRef &layerId,
                     const nonstd::optional<int> &index,
                     const nonstd::optional<int> &filterIndex,
                     const LayerChangeFunction &changeFunction) {
    octopus::LayerChange layerChange;
    layerChange.subject = subject;
    layerChange.op = operation;
    layerChange.index = index;
    layerChange.filterIndex = filterIndex;
    changeFunction(layerChange.values);

    std::string layerChangeStr;
    octopus::Serializer::serialize(layerChangeStr, layerChange);

    ODE_ParseError parseError { ODE_ParseError::Type::OK, 0 };
    const ODE_Result result = ode_component_modifyLayer(apiContext.component, layerId, ode_stringRef(layerChangeStr), &parseError);

    if (parseError.type != ODE_ParseError::OK) {
        return -1;
    }
    if (result != ODE_Result::ODE_RESULT_OK) {
        return result;
    }

    return ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
}

int changeProperty(octopus::LayerChange::Subject subject,
                   DesignEditorContext::Api &apiContext,
                   const ODE_StringRef &layerId,
                   const nonstd::optional<int> &index,
                   const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::PROPERTY_CHANGE, apiContext, layerId, index, nonstd::nullopt, changeFunction);
}

int changeInsertBack(octopus::LayerChange::Subject subject,
                     DesignEditorContext::Api &apiContext,
                     const ODE_StringRef &layerId,
                     const nonstd::optional<int> &index,
                     const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::INSERT, apiContext, layerId, index, nonstd::nullopt, changeFunction);
}

int changeReplace(octopus::LayerChange::Subject subject,
                  DesignEditorContext::Api &apiContext,
                  const ODE_StringRef &layerId,
                  const nonstd::optional<int> &index,
                  const nonstd::optional<int> &filterIndex,
                  const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REPLACE, apiContext, layerId, index, filterIndex, changeFunction);
}

int changeRemove(octopus::LayerChange::Subject subject,
                 DesignEditorContext::Api &apiContext,
                 const ODE_StringRef &layerId,
                 const nonstd::optional<int> &index,
                 const nonstd::optional<int> &filterIndex) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REMOVE, apiContext, layerId, index, filterIndex, NO_LAYER_CHANGE);
}

ImVec4 toImColor(const octopus::Color &color) {
    return ImVec4 {
        static_cast<float>(color.r),
        static_cast<float>(color.g),
        static_cast<float>(color.b),
        static_cast<float>(color.a),
    };
}

octopus::Color toOctopusColor(const ImVec4 &color) {
    return octopus::Color {
        static_cast<double>(color.x),
        static_cast<double>(color.y),
        static_cast<double>(color.z),
        static_cast<double>(color.w),
    };
}

}

std::string layerPropName(const ODE_StringRef &layerId,
                          const char *invisibleId,
                          const nonstd::optional<int> &index = nonstd::nullopt,
                          const nonstd::optional<int> &filterIndex = nonstd::nullopt,
                          const char *visibleLabel = "") {
    return
        std::string(visibleLabel) +
        std::string("##layer-")+std::string(invisibleId)+std::string("-")+ode_stringDeref(layerId) +
        (index.has_value() ? "-" + std::to_string(*index) : "") +
        (filterIndex.has_value() ? "-" + std::to_string(*filterIndex) : "");
};

void drawLayerInfo(const ODE_LayerList::Entry &layer) {
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
}

void drawLayerCommonProperties(const ODE_StringRef &layerId,
                               const octopus::Layer &octopusLayer,
                               DesignEditorContext::Api &apiContext) {
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool layerVisible = octopusLayer.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "visibility").c_str(), &layerVisible)) {
        changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layerId, nonstd::nullopt, [layerVisible](octopus::LayerChange::Values &values) {
            values.visible = layerVisible;
        });
    }

    ImGui::Text("Opacity:");
    ImGui::SameLine(100);
    float layerOpacity = octopusLayer.opacity;
    if (ImGui::SliderFloat(layerPropName(layerId, "opacity").c_str(), &layerOpacity, 0.0f, 1.0f)) {
        changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layerId, nonstd::nullopt, [layerOpacity](octopus::LayerChange::Values &values) {
            values.opacity = layerOpacity;
        });
    }

    ImGui::Text("Blend mode:");
    ImGui::SameLine(100);
    const int blendModeI = static_cast<int>(octopusLayer.blendMode);
    if (ImGui::BeginCombo(layerPropName(layerId, "blend-mode").c_str(), BLEND_MODES_STR[blendModeI])) {
        for (int bmI = 0; bmI < IM_ARRAYSIZE(BLEND_MODES_STR); bmI++) {
            const bool isSelected = (blendModeI == bmI);
            if (ImGui::Selectable(BLEND_MODES_STR[bmI], isSelected)) {
                changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layerId, nonstd::nullopt, [bmI](octopus::LayerChange::Values &values) {
                    values.blendMode = static_cast<octopus::BlendMode>(bmI);
                });
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerTransformation(const ODE_StringRef &layerId,
                             DesignEditorContext::Api &apiContext,
                             const ODE_LayerMetrics &layerMetrics) {
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

    ImGui::Text("Translation:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat2(layerPropName(layerId, "translation").c_str(), &translation.x, 1.0f)) {
        const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
        if (ode_component_transformLayer(apiContext.component, layerId, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
        }
    }

    ImGui::Text("Scale:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat2(layerPropName(layerId, "blend-scale").c_str(), &scale.x, 0.05f, 0.0f, 100.0f)) {
        const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
        if (ode_component_transformLayer(apiContext.component, layerId, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
        }
    }

    ImGui::Text("Rotation:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat(layerPropName(layerId, "blend-rotation").c_str(), &rotation)) {
        const float rotationChangeRad = -(rotation-origRotation)*M_PI/180.0f;
        const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
        if (ode_component_transformLayer(apiContext.component, layerId, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
        }
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerText(const ODE_StringRef &layerId,
                   DesignEditorContext::Api &apiContext,
                   const octopus::Text &octopusText) {
    const octopus::TextStyle &defaultTextStyle = octopusText.defaultStyle;

    // Text value
    char textBuffer[50] {};
    strncpy(textBuffer, octopusText.value.c_str(), sizeof(textBuffer)-1);
    ImGui::Text("Text value:");
    ImGui::SameLine(100);
    ImGui::InputText(layerPropName(layerId, "text-value").c_str(), textBuffer, 50);
    if (ImGui::IsItemEdited()) {
        changeProperty(octopus::LayerChange::Subject::TEXT, apiContext, layerId, nonstd::nullopt, [&textBuffer](octopus::LayerChange::Values &values) {
            values.value = textBuffer;
        });
    }

    // Font size
    float fontSize = defaultTextStyle.fontSize.has_value() ? *defaultTextStyle.fontSize : 0.0;
    ImGui::Text("Font size:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat(layerPropName(layerId, "text-font-size").c_str(), &fontSize, 0.1f, 0.0f, 100.0f)) {
        changeProperty(octopus::LayerChange::Subject::TEXT, apiContext, layerId, nonstd::nullopt, [&fontSize, &defaultTextStyle](octopus::LayerChange::Values &values) {
            values.defaultStyle = defaultTextStyle;
            values.defaultStyle->fontSize = fontSize;
        });
    }

    // Color
    ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    if (defaultTextStyle.fills.has_value() &&
        defaultTextStyle.fills->size() == 1 &&
        defaultTextStyle.fills->front().type == octopus::Fill::Type::COLOR &&
        defaultTextStyle.fills->front().color.has_value()) {
        imColor = toImColor(*defaultTextStyle.fills->front().color);
    }
    ImGui::Text("Color:");
    ImGui::SameLine(100);
    if (ImGui::ColorPicker4(layerPropName(layerId, "text-color").c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
        changeProperty(octopus::LayerChange::Subject::TEXT, apiContext, layerId, nonstd::nullopt, [&imColor, &defaultTextStyle](octopus::LayerChange::Values &values) {
            octopus::Fill newFill;
            newFill.type = octopus::Fill::Type::COLOR;
            newFill.color = toOctopusColor(imColor);
            values.defaultStyle = defaultTextStyle;
            values.defaultStyle->fills = std::vector<octopus::Fill> { newFill };
        });
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerShapeStroke(int strokeI,
                          const ODE_StringRef &layerId,
                          DesignEditorContext::Api &apiContext,
                          const octopus::Shape::Stroke &octopusShapeStroke) {
    const octopus::Fill &octopusShapeStrokeFill = octopusShapeStroke.fill;

    // Visibility
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool strokeVisible = octopusShapeStroke.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "stroke-visibility").c_str(), &strokeVisible)) {
        changeReplace(octopus::LayerChange::Subject::STROKE, apiContext, layerId, strokeI, nonstd::nullopt, [&octopusShapeStroke, strokeVisible](octopus::LayerChange::Values &values) {
            values.stroke = octopusShapeStroke;
            values.stroke->visible = strokeVisible;
        });
    }

    // Thickness
    ImGui::Text("Thickess:");
    ImGui::SameLine(100);
    float strokeThickness = octopusShapeStroke.thickness;
    if (ImGui::DragFloat(layerPropName(layerId, "shape-stroke-thickness").c_str(), &strokeThickness, 0.1f, 0.0f, 100.0f)) {
        changeReplace(octopus::LayerChange::Subject::STROKE, apiContext, layerId, strokeI, nonstd::nullopt, [&strokeThickness, &octopusShapeStroke](octopus::LayerChange::Values &values) {
            values.stroke = octopusShapeStroke;
            values.stroke->thickness = strokeThickness;
        });
    }

    // Position
    ImGui::Text("Position:");
    ImGui::SameLine(100);
    const int strokePositionI = static_cast<int>(octopusShapeStroke.position);
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-stroke-position").c_str(), STROKE_POSITIONS_STR[strokePositionI])) {
        for (int spI = 0; spI < IM_ARRAYSIZE(STROKE_POSITIONS_STR); spI++) {
            const bool isSelected = (strokePositionI == spI);
            if (ImGui::Selectable(STROKE_POSITIONS_STR[spI], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::STROKE, apiContext, layerId, strokeI, nonstd::nullopt, [spI, &octopusShapeStroke](octopus::LayerChange::Values &values) {
                    values.stroke = octopusShapeStroke;
                    values.stroke->position = static_cast<octopus::Stroke::Position>(spI);
                });
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Style (dashing)
    ImGui::Text("Dashing:");
    ImGui::SameLine(100);
    // TODO: What if shape stroke style is nullopt?
    const int strokeStyleI = static_cast<int>(octopusShapeStroke.style.has_value() ? *octopusShapeStroke.style : octopus::VectorStroke::Style::SOLID);
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-stroke-style").c_str(), STROKE_STYLES_STR[strokeStyleI])) {
        for (int ssI = 0; ssI < IM_ARRAYSIZE(STROKE_STYLES_STR); ssI++) {
            const bool isSelected = (strokeStyleI == ssI);
            if (ImGui::Selectable(STROKE_STYLES_STR[ssI], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::STROKE, apiContext, layerId, strokeI, nonstd::nullopt, [ssI, &octopusShapeStroke](octopus::LayerChange::Values &values) {
                    values.stroke = octopusShapeStroke;
                    values.stroke->style = static_cast<octopus::VectorStroke::Style>(ssI);
                });
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Color
    // TODO: What if shape stroke type other than COLOR?
    ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    if (octopusShapeStrokeFill.type == octopus::Fill::Type::COLOR && octopusShapeStrokeFill.color.has_value()) {
        imColor = toImColor(*octopusShapeStrokeFill.color);
    }
    ImGui::Text("Color:");
    ImGui::SameLine(100);
    if (ImGui::ColorPicker4(layerPropName(layerId, "shape-stroke-color").c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
        changeReplace(octopus::LayerChange::Subject::STROKE_FILL, apiContext, layerId, strokeI, nonstd::nullopt, [&imColor, &octopusShapeStrokeFill](octopus::LayerChange::Values &values) {
            values.fill = octopusShapeStrokeFill;
            values.fill->type = octopus::Fill::Type::COLOR;
            values.fill->color = toOctopusColor(imColor);
        });
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerShapeFill(int fillI,
                        const ODE_StringRef &layerId,
                        DesignEditorContext::Api &apiContext,
                        const octopus::Fill &octopusFill) {
    // Visibility
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool fillVisible = octopusFill.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "fill-visibility", fillI).c_str(), &fillVisible)) {
        changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, fillVisible](octopus::LayerChange::Values &values) {
            values.fill = octopusFill;
            values.fill->visible = fillVisible;
        });
    }

    // Blend mode
    ImGui::Text("Blend mode:");
    ImGui::SameLine(100);
    const int blendModeI = static_cast<int>(octopusFill.blendMode);
    if (ImGui::BeginCombo(layerPropName(layerId, "fill-blend-mode", fillI).c_str(), BLEND_MODES_STR[blendModeI])) {
        for (int bmI = 0; bmI < IM_ARRAYSIZE(BLEND_MODES_STR); bmI++) {
            const bool isSelected = (blendModeI == bmI);
            if (ImGui::Selectable(BLEND_MODES_STR[bmI], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, bmI](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    values.fill->blendMode = static_cast<octopus::BlendMode>(bmI);
                });
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Type
    ImGui::Text("Type:");
    ImGui::SameLine(100);
    const int fillTypeI = static_cast<int>(octopusFill.type);
    if (ImGui::BeginCombo(layerPropName(layerId, "fill-type", fillI).c_str(), FILL_TYPES_STR[fillTypeI])) {
        for (int ftI = 0; ftI < IM_ARRAYSIZE(FILL_TYPES_STR); ftI++) {
            const bool isSelected = (fillTypeI == ftI);
            if (ImGui::Selectable(FILL_TYPES_STR[ftI], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, ftI](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    values.fill->type = static_cast<octopus::Fill::Type>(ftI);
                });
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    switch (octopusFill.type) {
        case octopus::Fill::Type::COLOR:
        {
            // Color
            ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            if (octopusFill.color.has_value()) {
                imColor = toImColor(*octopusFill.color);
            }
            ImGui::Text("Color:");
            ImGui::SameLine(100);
            if (ImGui::ColorPicker4(layerPropName(layerId, "shape-fill-color", fillI).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&imColor, &octopusFill](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    values.fill->type = octopus::Fill::Type::COLOR;
                    values.fill->color = toOctopusColor(imColor);
                });
            }
            break;
        }
        case octopus::Fill::Type::GRADIENT:
        {
            // Gradient
            // TODO: Gradient stops?
            ImGui::Text("Gradient:");
            ImGui::SameLine(100);

            // TODO: What if fill gradient does not have a value?
            const int gradientTypeI = static_cast<int>(octopusFill.gradient.has_value() ? octopusFill.gradient->type : octopus::Gradient::Type::LINEAR);
            if (ImGui::BeginCombo(layerPropName(layerId, "fill-gradient-type", fillI).c_str(), FILL_GRADIENT_TYPES_STR[gradientTypeI])) {
                for (int gtI = 0; gtI < IM_ARRAYSIZE(FILL_GRADIENT_TYPES_STR); gtI++) {
                    const bool isSelected = (gradientTypeI == gtI);
                    if (ImGui::Selectable(FILL_GRADIENT_TYPES_STR[gtI], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, gtI](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->gradient.has_value()) {
                                values.fill->gradient->type = static_cast<octopus::Gradient::Type>(gtI);
                            } else {
                                values.fill->gradient = octopus::Gradient { static_cast<octopus::Gradient::Type>(gtI),
                                    std::vector<octopus::Gradient::ColorStop> {}
                                };
                            }
                        });
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // TODO: Gradient Color stops?
            break;
        }
        case octopus::Fill::Type::IMAGE:
        {
            ImGui::Text("Image:");

            // Image ref value
            ImGui::Text("  Ref type:");
            ImGui::SameLine(100);

            const int imageRefTypeI = static_cast<int>(octopusFill.image.has_value() ? octopusFill.image->ref.type : octopus::ImageRef::Type::PATH);
            if (ImGui::BeginCombo(layerPropName(layerId, "fill-image-ref-type", fillI).c_str(), IMAGE_REF_TYPES_STR[imageRefTypeI])) {
                for (int irI = 0; irI < IM_ARRAYSIZE(IMAGE_REF_TYPES_STR); irI++) {
                    const bool isSelected = (imageRefTypeI == irI);
                    if (ImGui::Selectable(IMAGE_REF_TYPES_STR[irI], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, irI](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->image.has_value()) {
                                values.fill->image->ref.type = static_cast<octopus::ImageRef::Type>(irI);
                            } else {
                                values.fill->image = octopus::Image {
                                    octopus::ImageRef { static_cast<octopus::ImageRef::Type>(irI), "" },
                                    nonstd::nullopt
                                };
                            }
                        });
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Image ref value
            const std::string &imageRefValue = octopusFill.image.has_value() ? octopusFill.image->ref.value : "";
            char textBuffer[50] {};
            strncpy(textBuffer, imageRefValue.c_str(), sizeof(textBuffer)-1);
            ImGui::Text("  Ref value:");
            ImGui::SameLine(100);
            ImGui::InputText(layerPropName(layerId, "fill-image-ref-value", fillI).c_str(), textBuffer, 50);
            if (ImGui::IsItemEdited()) {
                changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, &textBuffer](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    if (values.fill->image.has_value()) {
                        values.fill->image->ref.value = textBuffer;
                    } else {
                        values.fill->image = octopus::Image {
                            octopus::ImageRef { octopus::ImageRef::Type::PATH,  textBuffer},
                            nonstd::nullopt
                        };
                    }
                });
            }

            // TODO: Image ref subsection?

            // Image positioning layout
            ImGui::Text("  Pos. Layout:");
            ImGui::SameLine(100);

            const int positioningLayoutI = static_cast<int>(octopusFill.positioning.has_value() ? octopusFill.positioning->layout : octopus::Fill::Positioning::Layout::STRETCH);
            if (ImGui::BeginCombo(layerPropName(layerId, "fill-positioning-layout", fillI).c_str(), FILL_POSITIONING_LAYOUTS_STR[positioningLayoutI])) {
                for (int plI = 0; plI < IM_ARRAYSIZE(FILL_POSITIONING_LAYOUTS_STR); plI++) {
                    const bool isSelected = (positioningLayoutI == plI);
                    if (ImGui::Selectable(FILL_POSITIONING_LAYOUTS_STR[plI], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, plI](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->positioning.has_value()) {
                                values.fill->positioning->layout = static_cast<octopus::Fill::Positioning::Layout>(plI);
                            } else {
                                values.fill->positioning = octopus::Fill::Positioning { static_cast<octopus::Fill::Positioning::Layout>(plI), octopus::Fill::Positioning::Origin::LAYER,
                                };
                            }
                        });
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Image positioning origin
            ImGui::Text("  Pos. Orig.:");
            ImGui::SameLine(100);

            const int positioningOriginI = static_cast<int>(octopusFill.positioning.has_value() ? octopusFill.positioning->origin : octopus::Fill::Positioning::Origin::LAYER);
            if (ImGui::BeginCombo(layerPropName(layerId, "fill-positioning-origin", fillI).c_str(), FILL_POSITIONING_ORIGINS_STR[positioningOriginI])) {
                for (int poI = 0; poI < IM_ARRAYSIZE(FILL_POSITIONING_ORIGINS_STR); poI++) {
                    const bool isSelected = (positioningOriginI == poI);
                    if (ImGui::Selectable(FILL_POSITIONING_ORIGINS_STR[poI], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, apiContext, layerId, fillI, nonstd::nullopt, [&octopusFill, poI](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->positioning.has_value()) {
                                values.fill->positioning->origin = static_cast<octopus::Fill::Positioning::Origin>(poI);
                            } else {
                                values.fill->positioning = octopus::Fill::Positioning { octopus::Fill::Positioning::Layout::STRETCH, static_cast<octopus::Fill::Positioning::Origin>(poI),
                                };
                            }
                        });
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // TODO: Positioning transform?

            break;
        }
    }

    // TODO: Fill filters - only turn on / off ?
    if (octopusFill.filters.has_value()) {
        ImGui::Text("Filters:");
        for (int ffI = 0; ffI < static_cast<int>(octopusFill.filters->size()); ffI++) {
            const octopus::Filter &fillFilter = (*octopusFill.filters)[ffI];

            ImGui::Text("  Filter #%i:", ffI);
            ImGui::SameLine(100);

            bool filterVisible = (*octopusFill.filters)[ffI].visible;
            if (ImGui::Checkbox(layerPropName(layerId, "fill-filter-visibility", fillI, ffI).c_str(), &filterVisible)) {
                changeReplace(octopus::LayerChange::Subject::FILL_FILTER, apiContext, layerId, fillI, ffI, [fillFilter, filterVisible](octopus::LayerChange::Values &values) {
                    values.filter = fillFilter;
                    values.filter->visible = filterVisible;
                });
            }
        }
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

// TODO: Unsupported Subject SHAPE
void drawLayerShape(const ODE_StringRef &layerId,
                    DesignEditorContext::Api &apiContext,
                    const octopus::Shape &octopusShape) {
    // Rounded corner radius (for rectangles)
    {
        const bool isRectangle = (octopusShape.path.has_value() && octopusShape.path->type == octopus::Path::Type::RECTANGLE);
        if (isRectangle) {
            const octopus::Path &octopusShapePath = *octopusShape.path;
            float cornerRadius = octopusShape.path->cornerRadius.has_value() ? *octopusShape.path->cornerRadius : 0.0f;
            ImGui::Text("Corner radius:");
            ImGui::SameLine(100);
            if (ImGui::DragFloat(layerPropName(layerId, "shape-rectangle-corner-radius").c_str(), &cornerRadius, 1.0f, 0.0f, 1000.0f)) {
                changeProperty(octopus::LayerChange::Subject::SHAPE, apiContext, layerId, nonstd::nullopt, [&cornerRadius, &octopusShapePath](octopus::LayerChange::Values &values) {
                    values.path = octopusShapePath;
                    values.path->cornerRadius = cornerRadius;
                });
            }
        }

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
    }

    // Shape stroke
    // TODO: What about multiple or no strokes ? -> INSERT / REPLACE / REMOVE
    if (octopusShape.strokes.size() == 1 && ImGui::CollapsingHeader("Shape stroke (TODO)")) {
        drawLayerShapeStroke(0, layerId, apiContext, octopusShape.strokes.front());
    }

    // Shape Fill
    // TODO: What about multiple or no fills ? -> INSERT / REPLACE / REMOVE
    if (octopusShape.fills.size() == 1 && ImGui::CollapsingHeader("Shape fill (TODO)")) {
        drawLayerShapeFill(0, layerId, apiContext, octopusShape.fills.front());
    }
}


// TODO: Unsupported subject EFFECT
void drawLayerEffects(const ODE_StringRef &layerId,
                      DesignEditorContext::Api &apiContext,
                      const std::vector<octopus::Effect> &octopusEffects) {
    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
    ImGui::SameLine(415);
    if (ImGui::SmallButton(layerPropName(layerId, "effect-add", nonstd::nullopt, nonstd::nullopt, "+").c_str())) {
        changeInsertBack(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, nonstd::nullopt, [](octopus::LayerChange::Values &values) {
            octopus::Effect newEffect;
            newEffect.type = octopus::Effect::Type::OVERLAY;
            newEffect.overlay = DEFAULT_FILL;
            values.effect = newEffect;
        });
    }
    int effectToRemove = -1;
    for (size_t ei = 0; ei < octopusEffects.size(); ++ei) {
        const octopus::Effect &octopusEffect = octopusEffects[ei];

        ImGui::Text("Effect #%i:", static_cast<int>(ei));

        ImGui::SameLine(415);
        if (ImGui::SmallButton(layerPropName(layerId, "layer-effect-remove", ei, nonstd::nullopt, "-").c_str())) {
            effectToRemove = static_cast<int>(ei);
        }

        // Visibility
        ImGui::Text("Visible:");
        ImGui::SameLine(100);
        bool effectVisible = octopusEffect.visible;
        if (ImGui::Checkbox(layerPropName(layerId, "effect-visibility", ei).c_str(), &effectVisible)) {
            changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, effectVisible](octopus::LayerChange::Values &values) {
                values.effect = octopusEffect;
                values.effect->visible = effectVisible;
            });
        }

        ImGui::Text("Blend mode:");
        ImGui::SameLine(100);
        const int blendModeI = static_cast<int>(octopusEffect.blendMode);
        if (ImGui::BeginCombo(layerPropName(layerId, "effect-blend-mode", ei).c_str(), BLEND_MODES_STR[blendModeI])) {
            for (int bmI = 0; bmI < IM_ARRAYSIZE(BLEND_MODES_STR); bmI++) {
                const bool isSelected = (blendModeI == bmI);
                if (ImGui::Selectable(BLEND_MODES_STR[bmI], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, bmI](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->blendMode = static_cast<octopus::BlendMode>(bmI);
                    });
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Basis
        ImGui::Text("Basis:");
        ImGui::SameLine(100);
        const int effectBasisI = static_cast<int>(octopusEffect.basis);
        if (ImGui::BeginCombo(layerPropName(layerId, "effect-basis", ei).c_str(), EFFECT_BASIS_MAP.at(effectBasisI))) {
            for (const auto &ebPair : EFFECT_BASIS_MAP) {
                const int ebI = ebPair.first;
                const bool isSelected = (effectBasisI == ebI);
                if (ImGui::Selectable(EFFECT_BASIS_MAP.at(ebI), isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, ebI](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->basis = static_cast<octopus::EffectBasis>(ebI);
                    });
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Type
        ImGui::Text("Type:");
        ImGui::SameLine(100);
        const int effectTypeI = static_cast<int>(octopusEffect.type);
        if (ImGui::BeginCombo(layerPropName(layerId, "effect-type", ei).c_str(), EFFECT_TYPES_STR[effectTypeI])) {
            for (int etI = 0; etI < IM_ARRAYSIZE(EFFECT_TYPES_STR); etI++) {
                const bool isSelected = (effectTypeI == etI);
                if (ImGui::Selectable(EFFECT_TYPES_STR[etI], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, etI](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->type = static_cast<octopus::Effect::Type>(etI);
                        // TODO: Fill in reasonable default values when a type changes to a previously unused type
                    });
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        switch (octopusEffect.type) {
            case octopus::Effect::Type::OVERLAY:
            {
                if (!octopusEffect.overlay.has_value()) {
                    // TODO: Can overlay be undefined ?
                    break;
                }

                const octopus::Fill &octopusEffectOverlay = *octopusEffect.overlay;
                if (octopusEffectOverlay.type != octopus::Fill::Type::COLOR) {
                    // TODO: Handle other effect overlay types ?
                    break;
                }

                // Color
                ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                if (octopusEffectOverlay.color.has_value()) {
                    imColor = toImColor(*octopusEffectOverlay.color);
                }
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-overlay-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->overlay->type = octopus::Fill::Type::COLOR;
                        values.effect->overlay->color = toOctopusColor(imColor);
                    });
                }
                break;
            }
            case octopus::Effect::Type::STROKE:
            {
                if (!octopusEffect.stroke.has_value()) {
                    // TODO: Can stroke be undefined ?
                    break;
                }

                const octopus::Stroke &octopusEffectStroke = *octopusEffect.stroke;
                const octopus::Fill &octopusEffectStrokeFill = octopusEffectStroke.fill;

                // Thickness
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float strokeThickness = octopusEffectStroke.thickness;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-stroke-thickness", ei).c_str(), &strokeThickness, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&strokeThickness, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->stroke->thickness = strokeThickness;
                    });
                }

                // Position
                ImGui::Text("Position:");
                ImGui::SameLine(100);
                const int strokePositionI = static_cast<int>(octopusEffectStroke.position);
                if (ImGui::BeginCombo(layerPropName(layerId, "effect-stroke-position", ei).c_str(), STROKE_POSITIONS_STR[strokePositionI])) {
                    for (int spI = 0; spI < IM_ARRAYSIZE(STROKE_POSITIONS_STR); spI++) {
                        const bool isSelected = (strokePositionI == spI);
                        if (ImGui::Selectable(STROKE_POSITIONS_STR[spI], isSelected)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei,nonstd::nullopt,  [spI, &octopusEffect](octopus::LayerChange::Values &values) {
                                values.effect = octopusEffect;
                                values.effect->stroke->position = static_cast<octopus::Stroke::Position>(spI);
                            });
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                // Color
                // TODO: What if effect stroke type other than COLOR?
                ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                if (octopusEffectStrokeFill.type == octopus::Fill::Type::COLOR && octopusEffectStrokeFill.color.has_value()) {
                    imColor = toImColor(*octopusEffectStrokeFill.color);
                }
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-stroke-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->stroke->fill.type = octopus::Fill::Type::COLOR;
                        values.effect->stroke->fill.color = toOctopusColor(imColor);
                    });
                }

                break;
            }
            case octopus::Effect::Type::DROP_SHADOW:
            case octopus::Effect::Type::INNER_SHADOW:
            {
                if (!octopusEffect.shadow.has_value()) {
                    // TODO: Can shadow be undefined ?
                    break;
                }

                const octopus::Shadow &octopusEffectShadow = *octopusEffect.shadow;

                // Effect shadow offset
                ImGui::Text("Offset:");
                ImGui::SameLine(100);
                Vector2f shadowOffset { static_cast<float>(octopusEffectShadow.offset.x), static_cast<float>(octopusEffectShadow.offset.y) };
                if (ImGui::DragFloat2(layerPropName(layerId, "effect-shadow-offset", ei).c_str(), &shadowOffset.x, 0.05f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, &shadowOffset](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->offset.x = shadowOffset.x;
                        values.effect->shadow->offset.y = shadowOffset.y;
                    });
                }

                // Effect shadow blur
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float shadowBlur = octopusEffectShadow.blur;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-shadow-blur", ei).c_str(), &shadowBlur, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&shadowBlur, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->blur = shadowBlur;
                    });
                }

                // Effect shadow choke
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float shadowChoke = octopusEffectShadow.choke;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-shadow-choke", ei).c_str(), &shadowChoke, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&shadowChoke, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->choke = shadowChoke;
                    });
                }

                // Effect shadow color
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                const ImVec4 &imColor = toImColor(octopusEffectShadow.color);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-shadow-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->color = toOctopusColor(imColor);
                    });
                }
                break;
            }
            case octopus::Effect::Type::OUTER_GLOW:
            case octopus::Effect::Type::INNER_GLOW:
            {
                if (!octopusEffect.glow.has_value()) {
                    // TODO: Can glow be undefined ?
                    break;
                }

                const octopus::Shadow &octopusEffectGlow = *octopusEffect.glow;

                // Effect shadow offset
                ImGui::Text("Offset:");
                ImGui::SameLine(100);
                Vector2f shadowOffset { static_cast<float>(octopusEffectGlow.offset.x), static_cast<float>(octopusEffectGlow.offset.y) };
                if (ImGui::DragFloat2(layerPropName(layerId, "effect-glow-offset", ei).c_str(), &shadowOffset.x, 0.05f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, &shadowOffset](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->offset.x = shadowOffset.x;
                        values.effect->glow->offset.y = shadowOffset.y;
                    });
                }

                // Effect shadow blur
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float shadowBlur = octopusEffectGlow.blur;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-glow-blur", ei).c_str(), &shadowBlur, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&shadowBlur, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->blur = shadowBlur;
                    });
                }

                // Effect shadow choke
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float shadowChoke = octopusEffectGlow.choke;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-glow-choke", ei).c_str(), &shadowChoke, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&shadowChoke, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->choke = shadowChoke;
                    });
                }

                // Effect shadow color
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                const ImVec4 &imColor = toImColor(octopusEffectGlow.color);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-glow-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->color = toOctopusColor(imColor);
                    });
                }
                break;
            }
            case octopus::Effect::Type::GAUSSIAN_BLUR:
            case octopus::Effect::Type::BOUNDED_BLUR:
            case octopus::Effect::Type::BLUR:
            {
                if (!octopusEffect.blur.has_value()) {
                    // TODO: Can blur be undefined ?
                    break;
                }
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float blurAmount = *octopusEffect.blur;
                if (ImGui::DragFloat(layerPropName(layerId, "effect-blur-amount", ei).c_str(), &blurAmount, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, ei, nonstd::nullopt, [&octopusEffect, blurAmount](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->blur = blurAmount;
                    });
                }
                break;
            }
            case octopus::Effect::Type::OTHER:
            {
                break;
            }
        }

        // TODO: effect fill filters

    }
    if (effectToRemove >= 0 && effectToRemove < static_cast<int>(octopusEffects.size())) {
        changeRemove(octopus::LayerChange::Subject::EFFECT, apiContext, layerId, effectToRemove, nonstd::nullopt);
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerPropertiesWidget(const ODE_LayerList &layerList,
                               DesignEditorContext::Api &apiContext,
                               DesignEditorContext::LayerSelection &layerSelectionContext) {
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
            const octopus::Layer *octopusLayerPtr = findLayer(*componentOctopus.content, ode_stringDeref(layer.id));
            if (octopusLayerPtr == nullptr) {
                continue;
            }
            const octopus::Layer &octopusLayer = *octopusLayerPtr;

            ODE_LayerMetrics layerMetrics;
            if (ode_component_getLayerMetrics(apiContext.component, layer.id, &layerMetrics) != ODE_RESULT_OK) {
                continue;
            }

            const std::string layerSectionHeader =
                std::string("[")+layerTypeToShortString(layer.type)+std::string("] ")+
                ode_stringDeref(layer.id)+std::string(" ")+
                std::string("(")+ode_stringDeref(layer.name)+std::string(")");

            if (ImGui::CollapsingHeader(layerSectionHeader.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                // Layer info
                drawLayerInfo(layer);

                // Common layer properties
                drawLayerCommonProperties(layer.id, octopusLayer, apiContext);

                // Transformation
                drawLayerTransformation(layer.id, apiContext, layerMetrics);

                // Text
                const bool isTextLayer = (layer.type == ODE_LAYER_TYPE_TEXT && octopusLayer.type == octopus::Layer::Type::TEXT && octopusLayer.text.has_value());
                if (isTextLayer && ImGui::CollapsingHeader("Text")) {
                    drawLayerText(layer.id, apiContext, *octopusLayer.text);
                }

                // Shape
                const bool isShapeLayer = (layer.type == ODE_LAYER_TYPE_SHAPE && octopusLayer.type == octopus::Layer::Type::SHAPE && octopusLayer.shape.has_value());
                if (isShapeLayer) {
                    drawLayerShape(layer.id, apiContext, *octopusLayer.shape);
                }

                // Effects
                if (ImGui::CollapsingHeader("Effects (TODO)")) {
                    drawLayerEffects(layer.id, apiContext, octopusLayer.effects);
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_DARK_RED);
                ImGui::SameLine(100);
                if (ImGui::Button(layerPropName(layer.id, "delete", nonstd::nullopt, nonstd::nullopt, "Delete Layer [FUTURE_API]").c_str(), ImVec2 { 250, 20 })) {
                    // TODO: Remove layer when API available
//                    if (ode_component_removeLayer(apiContext.component, layer.id) == ODE_RESULT_OK) {
//                        ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
//                    }
                }
                ImGui::PopStyleColor(1);

                ImGui::Dummy(ImVec2 { 0.0f, 20.0f });
            }
        }
    }

    ImGui::End();
}
