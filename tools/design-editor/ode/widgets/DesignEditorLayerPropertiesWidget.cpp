
#include "DesignEditorLayerPropertiesWidget.h"

#include <imgui.h>

#include <octopus/layer-change.h>
#include <ode-essentials.h>

using namespace ode;

namespace {

const ImU32 IM_COLOR_DARK_RED = 4278190233;

// TODO: Cleanup:
const char *blendModesStr[] = {
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

const char *strokePositionsStr[] = {
    "OUTSIDE",
    "CENTER",
    "INSIDE",
};

const char *strokeStylesStr[] = {
    "SOLID",
    "DASHED",
    "DOTTED",
};

const char *fillGradientTypesStr[] = {
    "LINEAR",
    "RADIAL",
    "ANGULAR",
    "DIAMOND",
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
                     const nonstd::optional<int> &changeIndex,
                     const LayerChangeFunction &changeFunction) {
    octopus::LayerChange layerChange;
    layerChange.subject = subject;
    layerChange.op = operation;
    layerChange.index = changeIndex;
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
                   const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::PROPERTY_CHANGE, apiContext, layerId, nonstd::nullopt, changeFunction);
}

int changeInsertBack(octopus::LayerChange::Subject subject,
                 DesignEditorContext::Api &apiContext,
                 const ODE_StringRef &layerId,
                 const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::INSERT, apiContext, layerId, nonstd::nullopt, changeFunction);
}

int changeReplace(octopus::LayerChange::Subject subject,
                  DesignEditorContext::Api &apiContext,
                  const ODE_StringRef &layerId,
                  const nonstd::optional<int> &changeIndex,
                  const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REPLACE, apiContext, layerId, changeIndex, changeFunction);
}

int changeRemove(octopus::LayerChange::Subject subject,
                 DesignEditorContext::Api &apiContext,
                 const ODE_StringRef &layerId,
                 const nonstd::optional<int> &changeIndex) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REPLACE, apiContext, layerId, changeIndex, NO_LAYER_CHANGE);
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
            const auto layerPropName = [&layer](const char *invisibleId, const char *visibleLabel = "")->std::string {
                return std::string(visibleLabel)+std::string("##layer-")+std::string(invisibleId)+std::string("-")+ode_stringDeref(layer.id);
            };

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
                {
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

                // Common layer properties
                {
                    ImGui::Text("Visible:");
                    ImGui::SameLine(100);
                    bool layerVisible = octopusLayer.visible;
                    if (ImGui::Checkbox(layerPropName("visibility").c_str(), &layerVisible)) {
                        changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layer.id, [layerVisible](octopus::LayerChange::Values &values) {
                            values.visible = layerVisible;
                        });
                    }

                    ImGui::Text("Opacity:");
                    ImGui::SameLine(100);
                    float layerOpacity = octopusLayer.opacity;
                    if (ImGui::SliderFloat(layerPropName("opacity").c_str(), &layerOpacity, 0.0f, 1.0f)) {
                        changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layer.id, [layerOpacity](octopus::LayerChange::Values &values) {
                            values.opacity = layerOpacity;
                        });
                    }

                    ImGui::Text("Blend mode:");
                    ImGui::SameLine(100);
                    const int blendModeI = static_cast<int>(octopusLayer.blendMode);
                    if (ImGui::BeginCombo(layerPropName("blend-mode").c_str(), blendModesStr[blendModeI])) {
                        for (int bmI = 0; bmI < IM_ARRAYSIZE(blendModesStr); bmI++) {
                            const bool isSelected = (blendModeI == bmI);
                            if (ImGui::Selectable(blendModesStr[bmI], isSelected)) {
                                changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layer.id, [bmI](octopus::LayerChange::Values &values) {
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

                // Transformation
                {
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
                    if (ImGui::DragFloat2((std::string("##translation-")+ode_stringDeref(layer.id)).c_str(), &translation.x, 1.0f)) {
                        const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
                        if (ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
                            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
                        }
                    }

                    ImGui::Text("Scale:");
                    ImGui::SameLine(100);
                    if (ImGui::DragFloat2(layerPropName("blend-scale").c_str(), &scale.x, 0.05f, 0.0f, 100.0f)) {
                        const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
                        if (ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
                            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
                        }
                    }

                    ImGui::Text("Rotation:");
                    ImGui::SameLine(100);
                    if (ImGui::DragFloat(layerPropName("blend-rotation").c_str(), &rotation)) {
                        const float rotationChangeRad = -(rotation-origRotation)*M_PI/180.0f;
                        const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
                        if (ode_component_transformLayer(apiContext.component, layer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
                            ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView);
                        }
                    }

                    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                }

                // Text
                // TODO: Unsupported Subject TEXT
                const bool isTextLayer = (layer.type == ODE_LAYER_TYPE_TEXT && octopusLayer.type == octopus::Layer::Type::TEXT && octopusLayer.text.has_value());
                if (isTextLayer && ImGui::CollapsingHeader("Text")) {
                    const octopus::Text &octopusText = *octopusLayer.text;
                    const octopus::TextStyle &defaultTextStyle = octopusText.defaultStyle;

                    // Text value
                    char textBuffer[50] {};
                    strncpy(textBuffer, octopusText.value.c_str(), sizeof(textBuffer)-1);
                    ImGui::Text("Text value:");
                    ImGui::SameLine(100);
                    ImGui::InputText(layerPropName("text-value").c_str(), textBuffer, 50);
                    if (ImGui::IsItemEdited()) {
                        changeProperty(octopus::LayerChange::Subject::TEXT, apiContext, layer.id, [&textBuffer](octopus::LayerChange::Values &values) {
                            values.value = textBuffer;
                        });
                    }

                    // Font size
                    float fontSize = defaultTextStyle.fontSize.has_value() ? *defaultTextStyle.fontSize : 0.0;
                    ImGui::Text("Font size:");
                    ImGui::SameLine(100);
                    if (ImGui::DragFloat(layerPropName("text-font-size").c_str(), &fontSize, 0.1f, 0.0f, 100.0f)) {
                        changeProperty(octopus::LayerChange::Subject::LAYER, apiContext, layer.id, [&fontSize, &defaultTextStyle](octopus::LayerChange::Values &values) {
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
                    if (ImGui::ColorPicker4("text-color", (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                        changeProperty(octopus::LayerChange::Subject::TEXT, apiContext, layer.id, [&imColor, &defaultTextStyle](octopus::LayerChange::Values &values) {
                            octopus::Fill newFill;
                            newFill.type = octopus::Fill::Type::COLOR;
                            newFill.color = toOctopusColor(imColor);
                            values.defaultStyle = defaultTextStyle;
                            values.defaultStyle->fills = std::vector<octopus::Fill> { newFill };
                        });
                    }

                    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                }

                // Shape
                // TODO: unsupported Subject SHAPE
                const bool isShapeLayer = (layer.type == ODE_LAYER_TYPE_SHAPE && octopusLayer.type == octopus::Layer::Type::SHAPE && octopusLayer.shape.has_value());
                if (isShapeLayer) {
                    const octopus::Shape &octopusShape = *octopusLayer.shape;

                    // Rounded corner radius (for rectangles)
                    // TODO: Unsupported subject SHAPE
                    {
                        const bool isRectangle = (octopusShape.path.has_value() && octopusShape.path->type == octopus::Path::Type::RECTANGLE);
                        if (isRectangle) {
                            const octopus::Path &octopusShapePath = *octopusShape.path;
                            float cornerRadius = octopusShape.path->cornerRadius.has_value() ? *octopusShape.path->cornerRadius : 0.0f;
                            ImGui::Text("Corner radius:");
                            ImGui::SameLine(100);
                            if (ImGui::DragFloat(layerPropName("shape-rectangle-corner-radius").c_str(), &cornerRadius, 0.1f, 0.0f, 1000.0f)) {
                                changeProperty(octopus::LayerChange::Subject::SHAPE, apiContext, layer.id, [&cornerRadius, &octopusShapePath](octopus::LayerChange::Values &values) {
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
                        const octopus::Shape::Stroke &octopusShapeStroke = octopusShape.strokes.front();
                        const octopus::Fill &octopusShapeStrokeFill = octopusShapeStroke.fill;

                        // Visibility
                        ImGui::Text("Visible:");
                        ImGui::SameLine(100);
                        bool strokeVisible = octopusShapeStroke.visible;
                        if (ImGui::Checkbox(layerPropName("stroke-visibility").c_str(), &strokeVisible)) {
                            changeProperty(octopus::LayerChange::Subject::STROKE, apiContext, layer.id, [&octopusShapeStroke, strokeVisible](octopus::LayerChange::Values &values) {
                                values.stroke = octopusShapeStroke;
                                values.stroke->visible = strokeVisible;
                            });
                        }

                        // Thickness
                        ImGui::Text("Thickess:");
                        ImGui::SameLine(100);
                        float strokeThickness = octopusShapeStroke.thickness;
                        if (ImGui::DragFloat(layerPropName("shape-stroke-thickness").c_str(), &strokeThickness, 0.1f, 0.0f, 100.0f)) {
                            changeProperty(octopus::LayerChange::Subject::STROKE, apiContext, layer.id, [&strokeThickness, &octopusShapeStroke](octopus::LayerChange::Values &values) {
                                values.stroke = octopusShapeStroke;
                                values.stroke->thickness = strokeThickness;
                            });
                        }

                        // Position
                        ImGui::Text("Position:");
                        ImGui::SameLine(100);
                        const int strokePositionI = static_cast<int>(octopusShapeStroke.position);
                        if (ImGui::BeginCombo(layerPropName("shape-stroke-position").c_str(), strokePositionsStr[strokePositionI])) {
                            for (int spI = 0; spI < IM_ARRAYSIZE(strokePositionsStr); spI++) {
                                const bool isSelected = (strokePositionI == spI);
                                if (ImGui::Selectable(strokePositionsStr[spI], isSelected)) {
                                    changeProperty(octopus::LayerChange::Subject::STROKE, apiContext, layer.id, [spI, &octopusShapeStroke](octopus::LayerChange::Values &values) {
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
                        if (ImGui::BeginCombo(layerPropName("shape-stroke-style").c_str(), strokeStylesStr[strokeStyleI])) {
                            for (int ssI = 0; ssI < IM_ARRAYSIZE(strokeStylesStr); ssI++) {
                                const bool isSelected = (strokeStyleI == ssI);
                                if (ImGui::Selectable(strokeStylesStr[ssI], isSelected)) {
                                    changeProperty(octopus::LayerChange::Subject::STROKE, apiContext, layer.id, [ssI, &octopusShapeStroke](octopus::LayerChange::Values &values) {
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
                        ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                        if (octopusShapeStroke.fill.type == octopus::Fill::Type::COLOR && octopusShapeStroke.fill.color.has_value()) {
                            imColor = toImColor(*octopusShapeStroke.fill.color);
                        }
                        ImGui::Text("Color:");
                        ImGui::SameLine(100);
                        if (ImGui::ColorPicker4("shape-stroke-color", (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                            changeProperty(octopus::LayerChange::Subject::STROKE_FILL, apiContext, layer.id, [&imColor, &octopusShapeStrokeFill](octopus::LayerChange::Values &values) {
                                values.fill = octopusShapeStrokeFill;
                                values.fill->type = octopus::Fill::Type::COLOR;
                                values.fill->color = toOctopusColor(imColor);
                            });
                        }

                        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                    }

                    // Shape Fill
                    // TODO: What about multiple or no fills ? -> INSERT / REPLACE / REMOVE
                    if (octopusShape.fills.size() == 1 && ImGui::CollapsingHeader("Shape fill (TODO)")) {
                        const octopus::Fill &octopusFill = octopusShape.fills.front();

                        // Visibility
                        ImGui::Text("Visible:");
                        ImGui::SameLine(100);
                        bool fillVisible = octopusFill.visible;
                        if (ImGui::Checkbox(layerPropName("fill-visibility").c_str(), &fillVisible)) {
                            // TODO: Subject SHAPE or FILL ?
                            changeProperty(octopus::LayerChange::Subject::FILL, apiContext, layer.id, [&octopusFill, fillVisible](octopus::LayerChange::Values &values) {
                                values.fill = octopusFill;
                                values.fill->visible = fillVisible;
                            });
                        }

                        // Blend mode
                        ImGui::Text("Blend mode:");
                        ImGui::SameLine(100);
                        const int blendModeI = static_cast<int>(octopusFill.blendMode);
                        if (ImGui::BeginCombo(layerPropName("fill-blend-mode").c_str(), blendModesStr[blendModeI])) {
                            for (int bmI = 0; bmI < IM_ARRAYSIZE(blendModesStr); bmI++) {
                                const bool isSelected = (blendModeI == bmI);
                                if (ImGui::Selectable(blendModesStr[bmI], isSelected)) {
                                    changeProperty(octopus::LayerChange::Subject::FILL, apiContext, layer.id, [&octopusFill, bmI](octopus::LayerChange::Values &values) {
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

                        // Color
                        ImVec4 imColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                        if (octopusFill.type == octopus::Fill::Type::COLOR && octopusFill.color.has_value()) {
                            imColor = toImColor(*octopusFill.color);
                        }
                        ImGui::Text("Color:");
                        ImGui::SameLine(100);
                        if (ImGui::ColorPicker4("shape-fill-color", (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                            changeProperty(octopus::LayerChange::Subject::FILL, apiContext, layer.id, [&imColor, &octopusFill](octopus::LayerChange::Values &values) {
                                values.fill = octopusFill;
                                values.fill->type = octopus::Fill::Type::COLOR;
                                values.fill->color = toOctopusColor(imColor);
                            });
                        }

                        // Gradient
                        // TODO: Gradient stops?
                        ImGui::Text("Gradient type:");
                        ImGui::SameLine(100);
                        // TODO: What if fill gradient does not have a value?
                        const int gradientTypeI = static_cast<int>(octopusFill.gradient.has_value() ? octopusFill.gradient->type : octopus::Gradient::Type::LINEAR);
                        if (ImGui::BeginCombo(layerPropName("fill-gradient-type").c_str(), fillGradientTypesStr[gradientTypeI])) {
                            for (int gtI = 0; gtI < IM_ARRAYSIZE(fillGradientTypesStr); gtI++) {
                                const bool isSelected = (gradientTypeI == gtI);
                                if (ImGui::Selectable(fillGradientTypesStr[gtI], isSelected)) {
                                    changeProperty(octopus::LayerChange::Subject::FILL, apiContext, layer.id, [&octopusFill, gtI](octopus::LayerChange::Values &values) {
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

                        // TODO: Image + positioning
                        ImGui::Text("Image (TODO):");
                        ImGui::SameLine(100);
                        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                        // TODO: Fill filters
                        ImGui::Text("Filters (TODO):");
                        ImGui::SameLine(100);
                        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

                        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                    }
                }

                // Effects
                // TODO: Unsupported subject EFFECT
                if (ImGui::CollapsingHeader("Effects (TODO)")) {
                    const std::vector<octopus::Effect> &octopusEffects = octopusLayer.effects;

                    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                    ImGui::SameLine(415);
                    if (ImGui::SmallButton(layerPropName("effect-add", "+").c_str())) {
                        changeInsertBack(octopus::LayerChange::Subject::EFFECT, apiContext, layer.id, [](octopus::LayerChange::Values &values) {
                            // TODO: Default effect values?
                            octopus::Effect newEffect;
                            values.effect = newEffect;
                        });
                    }
                    int effectToRemove = -1;
                    for (size_t ei = 0; ei < octopusEffects.size(); ++ei) {
                        ImGui::Dummy(ImVec2(20.0f, 0.0f));
                        ImGui::SameLine(50);

                        bool effectVisible = octopusEffects[ei].visible;
                        if (ImGui::Checkbox(layerPropName("effect-visibility").c_str(), &effectVisible)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT, apiContext, layer.id, ei, [&effect = octopusEffects[ei], effectVisible](octopus::LayerChange::Values &values) {
                                values.effect = effect;
                                values.effect->visible = effectVisible;
                            });
                        }

                        ImGui::SameLine(415);
                        if (ImGui::SmallButton((std::string("-##layer-effect-remove")+std::to_string(ei)).c_str())) {
                            effectToRemove = static_cast<int>(ei);
                        }

                        // TODO: remaining effect parameters
                    }
                    if (effectToRemove >= 0 && effectToRemove < static_cast<int>(octopusEffects.size())) {
                        changeRemove(octopus::LayerChange::Subject::EFFECT, apiContext, layer.id, effectToRemove);
                    }

                    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                }

                ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_DARK_RED);
                ImGui::SameLine(100);
                if (ImGui::Button(layerPropName("delete", "Delete Layer [FUTURE_API]").c_str(), ImVec2 { 250, 20 })) {
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
