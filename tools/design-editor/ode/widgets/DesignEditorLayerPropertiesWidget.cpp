
#define _USE_MATH_DEFINES

#include "DesignEditorLayerPropertiesWidget.h"

#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <ImGuiFileDialog.h>

#include <octopus/layer-change.h>
#include <ode-essentials.h>

#include "DesignEditorUIHelpers.h"
#include "DesignEditorUIValues.h"

using namespace ode;

namespace {

class EnumMap : private std::vector<const char *> {
public:
    template <typename E>
    inline explicit EnumMap(E maxValue) : std::vector<const char *>(size_t(maxValue)+1, "???") { }
    template <typename E>
    const char *&operator[](E value) {
        if (size_t(value) >= size())
            resize(size_t(value)+1, "???");
        return std::vector<const char *>::operator[](size_t(value));
    }
    template <typename E>
    const char *operator[](E value) const {
        if (size_t(value) >= size())
            return "???";
        return std::vector<const char *>::operator[](size_t(value));
    }
    inline size_t size() const {
        return std::vector<const char *>::size();
    }
};

#define MAP_ENUM(ns, name) map[ns::name] = #name

static EnumMap makeBlendModeMap() {
    using octopus::BlendMode;
    EnumMap map(BlendMode::VIVID_LIGHT);
    MAP_ENUM(BlendMode, NORMAL);
    MAP_ENUM(BlendMode, PASS_THROUGH);
    MAP_ENUM(BlendMode, COLOR);
    MAP_ENUM(BlendMode, COLOR_BURN);
    MAP_ENUM(BlendMode, COLOR_DODGE);
    MAP_ENUM(BlendMode, DARKEN);
    MAP_ENUM(BlendMode, DARKER_COLOR);
    MAP_ENUM(BlendMode, DIFFERENCE);
    MAP_ENUM(BlendMode, DIVIDE);
    MAP_ENUM(BlendMode, EXCLUSION);
    MAP_ENUM(BlendMode, HARD_LIGHT);
    MAP_ENUM(BlendMode, HARD_MIX);
    MAP_ENUM(BlendMode, HUE);
    MAP_ENUM(BlendMode, LIGHTEN);
    MAP_ENUM(BlendMode, LIGHTER_COLOR);
    MAP_ENUM(BlendMode, LINEAR_BURN);
    MAP_ENUM(BlendMode, LINEAR_DODGE);
    MAP_ENUM(BlendMode, LINEAR_LIGHT);
    MAP_ENUM(BlendMode, LUMINOSITY);
    MAP_ENUM(BlendMode, MULTIPLY);
    MAP_ENUM(BlendMode, OVERLAY);
    MAP_ENUM(BlendMode, PIN_LIGHT);
    MAP_ENUM(BlendMode, SATURATION);
    MAP_ENUM(BlendMode, SCREEN);
    MAP_ENUM(BlendMode, SOFT_LIGHT);
    MAP_ENUM(BlendMode, SUBTRACT);
    MAP_ENUM(BlendMode, VIVID_LIGHT);
    return map;
}

static EnumMap makeStrokePositionMap() {
    using octopus::Stroke;
    EnumMap map(Stroke::Position::INSIDE);
    MAP_ENUM(Stroke::Position, OUTSIDE);
    MAP_ENUM(Stroke::Position, CENTER);
    MAP_ENUM(Stroke::Position, INSIDE);
    return map;
}

static EnumMap makeStrokeStyleMap() {
    using octopus::VectorStroke;
    EnumMap map(VectorStroke::Style::DOTTED);
    MAP_ENUM(VectorStroke::Style, SOLID);
    MAP_ENUM(VectorStroke::Style, DASHED);
    MAP_ENUM(VectorStroke::Style, DOTTED);
    return map;
}

static EnumMap makeFillTypeMap() {
    using octopus::Fill;
    EnumMap map(Fill::Type::IMAGE);
    MAP_ENUM(Fill::Type, COLOR);
    MAP_ENUM(Fill::Type, GRADIENT);
    MAP_ENUM(Fill::Type, IMAGE);
    return map;
}

static EnumMap makeGradientTypeMap() {
    using octopus::Gradient;
    EnumMap map(Gradient::Type::DIAMOND);
    MAP_ENUM(Gradient::Type, LINEAR);
    MAP_ENUM(Gradient::Type, RADIAL);
    MAP_ENUM(Gradient::Type, ANGULAR);
    MAP_ENUM(Gradient::Type, DIAMOND);
    return map;
}

static EnumMap makeFillPositioningLayoutMap() {
    using octopus::Fill;
    EnumMap map(Fill::Positioning::Layout::TILE);
    MAP_ENUM(Fill::Positioning::Layout, STRETCH);
    MAP_ENUM(Fill::Positioning::Layout, FILL);
    MAP_ENUM(Fill::Positioning::Layout, FIT);
    MAP_ENUM(Fill::Positioning::Layout, TILE);
    return map;
}

static EnumMap makeFillPositioningOriginMap() {
    using octopus::Fill;
    EnumMap map(Fill::Positioning::Origin::ARTBOARD);
    MAP_ENUM(Fill::Positioning::Origin, LAYER);
    MAP_ENUM(Fill::Positioning::Origin, PARENT);
    MAP_ENUM(Fill::Positioning::Origin, COMPONENT);
    MAP_ENUM(Fill::Positioning::Origin, ARTBOARD);
    return map;
}

static EnumMap makeImageRefTypeMap() {
    using octopus::ImageRef;
    EnumMap map(ImageRef::Type::RESOURCE_REF);
    MAP_ENUM(ImageRef::Type, PATH);
    MAP_ENUM(ImageRef::Type, RESOURCE_REF);
    return map;
}

static EnumMap makeEffectTypeMap() {
    using octopus::Effect;
    EnumMap map(Effect::Type::OTHER);
    MAP_ENUM(Effect::Type, OVERLAY);
    MAP_ENUM(Effect::Type, STROKE);
    MAP_ENUM(Effect::Type, DROP_SHADOW);
    MAP_ENUM(Effect::Type, INNER_SHADOW);
    MAP_ENUM(Effect::Type, OUTER_GLOW);
    MAP_ENUM(Effect::Type, INNER_GLOW);
    MAP_ENUM(Effect::Type, GAUSSIAN_BLUR);
    MAP_ENUM(Effect::Type, BOUNDED_BLUR);
    MAP_ENUM(Effect::Type, BLUR);
    MAP_ENUM(Effect::Type, OTHER);
    return map;
}

static const EnumMap BLEND_MODES = makeBlendModeMap();
static const EnumMap STROKE_POSITIONS = makeStrokePositionMap();
static const EnumMap STROKE_STYLES = makeStrokeStyleMap();
static const EnumMap FILL_TYPES = makeFillTypeMap();
static const EnumMap GRADIENT_TYPES = makeGradientTypeMap();
static const EnumMap FILL_POSITIONING_LAYOUTS = makeFillPositioningLayoutMap();
static const EnumMap FILL_POSITIONING_ORIGINS = makeFillPositioningOriginMap();
static const EnumMap IMAGE_REF_TYPES = makeImageRefTypeMap();
static const EnumMap EFFECT_TYPES = makeEffectTypeMap();

const std::map<int, const char*> EFFECT_BASIS_MAP {
    { 1, "BODY" },
    { 3, "BODY_AND_STROKES" },
    { 4, "FILL" },
    { 7, "LAYER_AND_EFFECTS" },
    { 8, "BACKGROUND" },
};

const octopus::Color COLOR_WHITE { 1.0f, 1.0f, 1.0f, 1.0f };
const octopus::Color COLOR_BLACK { 0.0f, 0.0f, 0.0f, 1.0f };
const octopus::Color COLOR_TRANSPARENT_BLACK { 0.0f, 0.0f, 0.0f, 0.0f };
const octopus::Color DEFAULT_FILL_COLOR = COLOR_WHITE;
const octopus::Color DEFAULT_STROKE_FILL_COLOR = COLOR_BLACK;
const octopus::Color DEFAULT_FILL_GRADIENT_COLOR_0 = COLOR_TRANSPARENT_BLACK;
const octopus::Color DEFAULT_FILL_GRADIENT_COLOR_1 = COLOR_BLACK;
const octopus::Color DEFAULT_EFFECT_COLOR = COLOR_BLACK;

const octopus::Gradient::ColorStop DEFAULT_FILL_GRADIENT_COLOR_STOP_0 {
    0.0,
    octopus::Gradient::Interpolation::LINEAR,
    0.0,
    DEFAULT_FILL_GRADIENT_COLOR_0 };
const octopus::Gradient::ColorStop DEFAULT_FILL_GRADIENT_COLOR_STOP_1 {
    1.0,
    octopus::Gradient::Interpolation::LINEAR,
    1.0,
    DEFAULT_FILL_GRADIENT_COLOR_1 };

const octopus::Gradient DEFAULT_FILL_GRADIENT {
    octopus::Gradient::Type::LINEAR,
    std::vector<octopus::Gradient::ColorStop> {
        DEFAULT_FILL_GRADIENT_COLOR_STOP_0,
        DEFAULT_FILL_GRADIENT_COLOR_STOP_1,
    },
};

const octopus::Image DEFAULT_FILL_IMAGE {
    octopus::ImageRef { octopus::ImageRef::Type::PATH, "" },
    nonstd::nullopt
};

const octopus::Fill DEFAULT_FILL {
    octopus::Fill::Type::COLOR,
    true,
    octopus::BlendMode::NORMAL,
    DEFAULT_FILL_COLOR,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt
};

const octopus::Stroke DEFAULT_STROKE {
    DEFAULT_FILL,
    2.0,
    octopus::Stroke::Position::CENTER
};

const octopus::Shape::Stroke DEFAULT_SHAPE_STROKE {
    { DEFAULT_STROKE,
        true,
        octopus::Shape::Stroke::Style::SOLID,
        nonstd::nullopt,
        nonstd::nullopt,
        nonstd::nullopt,
        nonstd::nullopt,
        nonstd::nullopt },
    nonstd::nullopt,
    nonstd::nullopt,
};

const octopus::Fill DEFAULT_EFFECT_FILL {
    octopus::Fill::Type::COLOR,
    true,
    octopus::BlendMode::NORMAL,
    DEFAULT_EFFECT_COLOR,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt
};

const octopus::Shadow DEFAULT_EFFECT_SHADOW {
    octopus::Vec2 { 0.0, 0.0 },
    5.0,
    5.0,
    DEFAULT_EFFECT_COLOR
};

const octopus::Shadow DEFAULT_EFFECT_GLOW {
    octopus::Vec2 { 0.0, 0.0 },
    5.0,
    5.0,
    DEFAULT_EFFECT_COLOR
};

const double DEFAULT_EFFECT_BLUR = 2.0;

const octopus::Effect DEFAULT_EFFECT {
    octopus::Effect::Type::OVERLAY,
    octopus::EffectBasis::FILL,
    true,
    octopus::BlendMode::NORMAL,
    DEFAULT_EFFECT_FILL,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
    nonstd::nullopt,
};

const octopus::Filter DEFAULT_FILL_FILTER {
    octopus::Filter::Type::OPACITY_MULTIPLIER,
    true,
    2.0,
    2.0,
    octopus::ColorAdjustment { 1.0,0,0,0,0,0,0 } // Adjust hue
};

using LayerChangeFunction = std::function<void(octopus::LayerChange::Values &)>;
const LayerChangeFunction NO_LAYER_CHANGE = [](octopus::LayerChange::Values &) {};

int applyLayerChange(octopus::LayerChange::Subject subject,
                     octopus::LayerChange::Op operation,
                     DesignEditorContext &context,
                     DesignEditorComponent &component,
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
    const ODE_Result result = ode_component_modifyLayer(component.component, layerId, ode_stringRef(layerChangeStr), &parseError);

    if (parseError.type != ODE_ParseError::OK) {
        return -1;
    }
    if (result != ODE_Result::ODE_RESULT_OK) {
        return result;
    }

    return ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
}

int changeProperty(octopus::LayerChange::Subject subject,
                   DesignEditorContext &context,
                   DesignEditorComponent &component,
                   const ODE_StringRef &layerId,
                   const nonstd::optional<int> &index,
                   const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::PROPERTY_CHANGE, context, component, layerId, index, nonstd::nullopt, changeFunction);
}

int changeInsertBack(octopus::LayerChange::Subject subject,
                     DesignEditorContext &context,
                     DesignEditorComponent &component,
                     const ODE_StringRef &layerId,
                     const nonstd::optional<int> &index,
                     const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::INSERT, context, component, layerId, index, nonstd::nullopt, changeFunction);
}

int changeReplace(octopus::LayerChange::Subject subject,
                  DesignEditorContext &context,
                  DesignEditorComponent &component,
                  const ODE_StringRef &layerId,
                  const nonstd::optional<int> &index,
                  const nonstd::optional<int> &filterIndex,
                  const LayerChangeFunction &changeFunction) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REPLACE, context, component, layerId, index, filterIndex, changeFunction);
}

int changeRemove(octopus::LayerChange::Subject subject,
                 DesignEditorContext &context,
                 DesignEditorComponent &component,
                 const ODE_StringRef &layerId,
                 const nonstd::optional<int> &index,
                 const nonstd::optional<int> &filterIndex) {
    return applyLayerChange(subject, octopus::LayerChange::Op::REMOVE, context, component, layerId, index, filterIndex, NO_LAYER_CHANGE);
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
}

std::string layerHeaderLabel(const char *label,
                             const ODE_StringRef &layerId) {
    return std::string(label) + "##" + ode_stringDeref(layerId);
}

}

void drawLayerInfo(const ODE_LayerList::Entry &layer,
                   DesignEditorContext &context,
                   DesignEditorComponent &component) {
    ImGui::Text("%s", "ID:");
    ImGui::SameLine(100);
    ImGui::Text("%s", layer.id.data);

    // Editable name
    ImGui::Text("%s", "Name:");
    ImGui::SameLine(100);
    char textBuffer[255] {};
    strncpy(textBuffer, layer.name.data, sizeof(textBuffer)-1);
    ImGui::InputText(layerPropName(layer.id, "layer-name").c_str(), textBuffer, 255);
    if (ImGui::IsItemEdited()) {
        changeProperty(octopus::LayerChange::Subject::LAYER, context, component, layer.id, nonstd::nullopt, [&textBuffer](octopus::LayerChange::Values &values) {
            values.name = textBuffer;
        });
        // Recreate the layer list, as the name of this layer has changed
        ode_component_listLayers(component.component, &component.layerList);
    }

    ImGui::Text("%s", "Type:");
    ImGui::SameLine(100);
    ImGui::Text("%s", layerTypeToString(layer.type).c_str());

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerCommonProperties(const ODE_StringRef &layerId,
                               const octopus::Layer &octopusLayer,
                               DesignEditorContext &context,
                               DesignEditorComponent &component) {
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool layerVisible = octopusLayer.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "layer-visibility").c_str(), &layerVisible)) {
        changeProperty(octopus::LayerChange::Subject::LAYER, context, component, layerId, nonstd::nullopt, [layerVisible](octopus::LayerChange::Values &values) {
            values.visible = layerVisible;
        });
    }

    ImGui::Text("Opacity:");
    ImGui::SameLine(100);
    float layerOpacity = float(octopusLayer.opacity);
    if (ImGui::SliderFloat(layerPropName(layerId, "layer-opacity").c_str(), &layerOpacity, 0.0f, 1.0f)) {
        changeProperty(octopus::LayerChange::Subject::LAYER, context, component, layerId, nonstd::nullopt, [layerOpacity](octopus::LayerChange::Values &values) {
            values.opacity = layerOpacity;
        });
    }

    ImGui::Text("Blend mode:");
    ImGui::SameLine(100);
    if (ImGui::BeginCombo(layerPropName(layerId, "layer-blend-mode").c_str(), BLEND_MODES[octopusLayer.blendMode])) {
        for (size_t i = 0; i < BLEND_MODES.size(); ++i) {
            const bool isSelected = i == size_t(octopusLayer.blendMode);
            if (ImGui::Selectable(BLEND_MODES[i], isSelected)) {
                changeProperty(octopus::LayerChange::Subject::LAYER, context, component, layerId, nonstd::nullopt, [i](octopus::LayerChange::Values &values) {
                    values.blendMode = static_cast<octopus::BlendMode>(i);
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
                             DesignEditorContext &context,
                             DesignEditorComponent &component,
                             const ODE_LayerMetrics &layerMetrics) {
    const double a = layerMetrics.transformation.matrix[0];
    const double b = layerMetrics.transformation.matrix[1];
    const double c = layerMetrics.transformation.matrix[2];
    const double d = layerMetrics.transformation.matrix[3];
    const double trX = layerMetrics.transformation.matrix[4];
    const double trY = layerMetrics.transformation.matrix[5];
    const double r = sqrt(a*a+b*b);
    const double s = sqrt(c*c+d*d);

    Vector2f translation { float(trX), float(trY) };
    Vector2f scale { float(r), float(s) };
    float rotation = float((b < 0 ? acos(a/r) : -acos(a/r)) * (180.0/M_PI));
    if (rotation < 0.0f) {
        rotation += 360.0f;
    }

    const Vector2f origTranslation = translation;
    const Vector2f origScale = scale;
    const float origRotation = rotation;

    ImGui::Text("Translation:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat2(layerPropName(layerId, "layer-translation").c_str(), &translation.x, 1.0f)) {
        const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
        if (ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT, newTransformation) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
        }
    }

    ImGui::Text("Scale:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat2(layerPropName(layerId, "layer-scale").c_str(), &scale.x, 0.05f, 0.0f, 100.0f)) {
        const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
        if (ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
        }
    }

    ImGui::Text("Rotation:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat(layerPropName(layerId, "layer-rotation").c_str(), &rotation)) {
        const double rotationChangeRad = (rotation-origRotation)*(M_PI/180.0);
        const ODE_Transformation trToOrigin { 1,0,0,1,-translation.x,-translation.y };
        const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
        const ODE_Transformation trFromOrigin { 1,0,0,1,translation.x,translation.y };

        if (ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT, trToOrigin) == ODE_RESULT_OK &&
            ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT, newTransformation) == ODE_RESULT_OK &&
            ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT, trFromOrigin) == ODE_RESULT_OK) {
            ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
        }
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerText(const ODE_StringRef &layerId,
                   DesignEditorContext &context,
                   DesignEditorComponent &component,
                   const octopus::Text &octopusText) {
    const octopus::TextStyle &defaultTextStyle = octopusText.defaultStyle;

    // Text value
    const std::string text = std::regex_replace(octopusText.value, std::regex("\xe2\x80\xa9"), "\n");
    char textBuffer[1<<16] {};
    strncpy(textBuffer, text.c_str(), sizeof(textBuffer)-1);
    ImGui::Text("Text value:");
    ImGui::SameLine(100);
    ImGui::InputTextMultiline(layerPropName(layerId, "text-value").c_str(), textBuffer, IM_ARRAYSIZE(textBuffer), ImVec2(327, ImGui::GetTextLineHeight() * 10), ImGuiInputTextFlags_AllowTabInput);
    if (ImGui::IsItemEdited()) {
        const std::string &oldText = text;
        const std::string newText = textBuffer;
        // Added and removed text start positions and lengths
        std::pair<int, int> removedText, addedText;

        for (size_t i = 0; i < std::max(oldText.size(), newText.size()); ++i) {
            if (i >= oldText.size()) {
                addedText = std::make_pair(oldText.size(), newText.size()-oldText.size());
                break;
            }
            if (i >= newText.size()) {
                removedText = std::make_pair(newText.size(), oldText.size()-newText.size());
                break;
            }
            if (oldText[i] == newText[i]) {
                continue;
            }
            const std::string remOldText(oldText.begin()+i, oldText.end());
            const std::string remNewText(newText.begin()+i, newText.end());
            const size_t addedEnd = remNewText.find(remOldText);
            const size_t removedEnd = remOldText.find(remNewText);
            if (addedEnd != std::string::npos) {
                addedText = std::make_pair(i, addedEnd);
                break;
            }
            if (removedEnd != std::string::npos) {
                removedText = std::make_pair(i, removedEnd);
                break;
            }
        }

        nonstd::optional<std::vector<octopus::StyleRange>> newStyles = octopusText.styles;
        if (newStyles.has_value()) {
            for (octopus::StyleRange &style : newStyles.value()) {
                for (octopus::StyleRange::Range &range : style.ranges) {
                    if (range.to > addedText.first) range.to += addedText.second;
                    if (range.from > addedText.first) range.from += addedText.second;
                    if (range.to > removedText.first) range.to -= removedText.second;
                    if (range.from > removedText.first) range.from -= removedText.second;
                }
            }
        }

        changeProperty(octopus::LayerChange::Subject::TEXT, context, component, layerId, nonstd::nullopt, [&textBuffer, &newStyles](octopus::LayerChange::Values &values) {
            values.value = textBuffer;
            values.styles = newStyles;
        });
    }

    // Font size
    float fontSize = float(defaultTextStyle.fontSize.has_value() ? *defaultTextStyle.fontSize : 0.0);
    ImGui::Text("Font size:");
    ImGui::SameLine(100);
    if (ImGui::DragFloat(layerPropName(layerId, "text-font-size").c_str(), &fontSize, 0.1f, 0.0f, 10000.0f)) {
        changeProperty(octopus::LayerChange::Subject::TEXT, context, component, layerId, nonstd::nullopt, [&fontSize, &defaultTextStyle](octopus::LayerChange::Values &values) {
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
        changeProperty(octopus::LayerChange::Subject::TEXT, context, component, layerId, nonstd::nullopt, [&imColor, &defaultTextStyle](octopus::LayerChange::Values &values) {
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
                          DesignEditorContext &context,
                          DesignEditorComponent &component,
                          const octopus::Shape::Stroke &octopusShapeStroke) {
    const octopus::Fill &octopusShapeStrokeFill = octopusShapeStroke.fill;

    // Visibility
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool strokeVisible = octopusShapeStroke.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "shape-stroke-visibility", strokeI).c_str(), &strokeVisible)) {
        changeReplace(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeI, nonstd::nullopt, [&octopusShapeStroke, strokeVisible](octopus::LayerChange::Values &values) {
            values.stroke = octopusShapeStroke;
            values.stroke->visible = strokeVisible;
        });
    }

    // Thickness
    ImGui::Text("Thickness:");
    ImGui::SameLine(100);
    float strokeThickness = float(octopusShapeStroke.thickness);
    if (ImGui::DragFloat(layerPropName(layerId, "shape-stroke-thickness", strokeI).c_str(), &strokeThickness, 0.1f, 0.0f, 100.0f)) {
        changeReplace(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeI, nonstd::nullopt, [&strokeThickness, &octopusShapeStroke](octopus::LayerChange::Values &values) {
            values.stroke = octopusShapeStroke;
            values.stroke->thickness = strokeThickness;
        });
    }

    // Position
    ImGui::Text("Position:");
    ImGui::SameLine(100);
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-stroke-position", strokeI).c_str(), STROKE_POSITIONS[octopusShapeStroke.position])) {
        for (size_t i = 0; i < STROKE_POSITIONS.size(); ++i) {
            const bool isSelected = i == size_t(octopusShapeStroke.position);
            if (ImGui::Selectable(STROKE_POSITIONS[i], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeI, nonstd::nullopt, [i, &octopusShapeStroke](octopus::LayerChange::Values &values) {
                    values.stroke = octopusShapeStroke;
                    values.stroke->position = static_cast<octopus::Stroke::Position>(i);
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
    const octopus::VectorStroke::Style strokeStyle = octopusShapeStroke.style.value_or(octopus::VectorStroke::Style::SOLID);
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-stroke-style", strokeI).c_str(), STROKE_STYLES[strokeStyle])) {
        for (size_t i = 0; i < STROKE_STYLES.size(); ++i) {
            const bool isSelected = i == size_t(strokeStyle);
            if (ImGui::Selectable(STROKE_STYLES[i], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeI, nonstd::nullopt, [i, &octopusShapeStroke](octopus::LayerChange::Values &values) {
                    values.stroke = octopusShapeStroke;
                    values.stroke->style = static_cast<octopus::VectorStroke::Style>(i);
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
    if (octopusShapeStrokeFill.type == octopus::Fill::Type::COLOR && octopusShapeStrokeFill.color.has_value()) {
        imColor = toImColor(*octopusShapeStrokeFill.color);
    }
    ImGui::Text("Color:");
    ImGui::SameLine(100);
    if (ImGui::ColorPicker4(layerPropName(layerId, "shape-stroke-color", strokeI).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
        changeProperty(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeI, [&imColor, &octopusShapeStrokeFill](octopus::LayerChange::Values &values) {
            values.fill = octopusShapeStrokeFill;
            values.fill->type = octopus::Fill::Type::COLOR;
            values.fill->color = toOctopusColor(imColor);
        });
    }

    // Stroke fill filters
    ImGui::Text("Fill filters:");
    ImGui::SameLine(415);
    if (ImGui::SmallButton(layerPropName(layerId, "stroke-fill-filter-add", strokeI, nonstd::nullopt, "+").c_str())) {
        changeInsertBack(octopus::LayerChange::Subject::STROKE_FILL_FILTER, context, component, layerId, strokeI, [](octopus::LayerChange::Values &values) {
            values.filter = DEFAULT_FILL_FILTER;
        });
    }

    if (octopusShapeStrokeFill.filters.has_value() && !octopusShapeStrokeFill.filters->empty()) {
        int strokeFillFilterToRemove = -1;

        for (int sffI = 0; sffI < static_cast<int>(octopusShapeStrokeFill.filters->size()); sffI++) {
            ImGui::Text("  Filter #%i:", sffI);
            ImGui::SameLine(100);

            const octopus::Filter &octopusEffectStrokeFillFilter = (*octopusShapeStrokeFill.filters)[sffI];

            bool strokeFillFilterVisible = octopusEffectStrokeFillFilter.visible;
            if (ImGui::Checkbox(layerPropName(layerId, "stroke-fill-filter-visibility", strokeI, sffI).c_str(), &strokeFillFilterVisible)) {
                changeReplace(octopus::LayerChange::Subject::STROKE_FILL_FILTER, context, component, layerId, strokeI, sffI, [&octopusEffectStrokeFillFilter, strokeFillFilterVisible](octopus::LayerChange::Values &values) {
                    values.filter = octopusEffectStrokeFillFilter;
                    values.filter->visible = strokeFillFilterVisible;
                });
            }

            ImGui::SameLine(415);
            if (ImGui::SmallButton(layerPropName(layerId, "stroke-fill-filter-remove", strokeI, sffI, "-").c_str())) {
                strokeFillFilterToRemove = static_cast<int>(sffI);
            }
        }

        if (strokeFillFilterToRemove >= 0 && strokeFillFilterToRemove < static_cast<int>(octopusShapeStrokeFill.filters->size())) {
            changeRemove(octopus::LayerChange::Subject::STROKE_FILL_FILTER, context, component, layerId, strokeI, strokeFillFilterToRemove);
        }
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerShapeFill(int fillI,
                        const ODE_StringRef &layerId,
                        DesignEditorContext &context,
                        DesignEditorComponent &component,
                        DesignEditorUIState::FileDialog &fileDialogContext,
                        const octopus::Fill &octopusFill,
                        const ODE_LayerMetrics &layerMetrics) {
    const double defaultGradientFillPositioningTransform[6] {
        0, layerMetrics.logicalBounds.b.y, -layerMetrics.logicalBounds.b.y, 0, layerMetrics.logicalBounds.b.x/2.0, 0
    };
    const double defaultImageFillPositioningTransform[6] {
        layerMetrics.logicalBounds.b.x, 0, 0, layerMetrics.logicalBounds.b.y, 0, 0
    };

    // Visibility
    ImGui::Text("Visible:");
    ImGui::SameLine(100);
    bool fillVisible = octopusFill.visible;
    if (ImGui::Checkbox(layerPropName(layerId, "shape-fill-visibility", fillI).c_str(), &fillVisible)) {
        changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, fillVisible](octopus::LayerChange::Values &values) {
            values.fill = octopusFill;
            values.fill->visible = fillVisible;
        });
    }

    // Blend mode
    ImGui::Text("Blend mode:");
    ImGui::SameLine(100);
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-blend-mode", fillI).c_str(), BLEND_MODES[octopusFill.blendMode])) {
        for (size_t i = 0; i < BLEND_MODES.size(); ++i) {
            const bool isSelected = i == size_t(octopusFill.blendMode);
            if (ImGui::Selectable(BLEND_MODES[i], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    values.fill->blendMode = static_cast<octopus::BlendMode>(i);
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
    if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-type", fillI).c_str(), FILL_TYPES[octopusFill.type])) {
        for (size_t i = 0; i < FILL_TYPES.size(); ++i) {
            const bool isSelected = i == size_t(octopusFill.type);
            if (ImGui::Selectable(FILL_TYPES[i], isSelected)) {
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i, &defaultGradientFillPositioningTransform, &defaultImageFillPositioningTransform](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    values.fill->type = static_cast<octopus::Fill::Type>(i);
                    switch (values.fill->type) {
                        case octopus::Fill::Type::COLOR:
                            if (!octopusFill.color.has_value()) {
                                values.fill->color = DEFAULT_FILL_COLOR;
                            }
                            break;
                        case octopus::Fill::Type::GRADIENT:
                            if (!octopusFill.gradient.has_value()) {
                                // Top to bottom transparent to black gradient
                                values.fill->gradient = DEFAULT_FILL_GRADIENT;
                                values.fill->positioning = octopus::Fill::Positioning {
                                    octopus::Fill::Positioning::Layout::STRETCH,
                                    octopus::Fill::Positioning::Origin::LAYER,
                                };
                                memcpy(values.fill->positioning->transform, defaultGradientFillPositioningTransform, sizeof(defaultGradientFillPositioningTransform));
                            }
                            break;
                        case octopus::Fill::Type::IMAGE:
                            if (!octopusFill.image.has_value()) {
                                values.fill->image = DEFAULT_FILL_IMAGE;
                                values.fill->positioning = octopus::Fill::Positioning {
                                    octopus::Fill::Positioning::Layout::STRETCH,
                                    octopus::Fill::Positioning::Origin::LAYER,
                                };
                                memcpy(values.fill->positioning->transform, defaultImageFillPositioningTransform, sizeof(defaultImageFillPositioningTransform));
                            }
                            break;
                    }
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
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&imColor, &octopusFill](octopus::LayerChange::Values &values) {
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
            ImGui::Text("Gradient:");
            ImGui::SameLine(100);

            const octopus::Gradient::Type gradientType = octopusFill.gradient.has_value() ? octopusFill.gradient->type : octopus::Gradient::Type::LINEAR;
            if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-gradient-type", fillI).c_str(), GRADIENT_TYPES[gradientType])) {
                for (size_t i = 0; i < GRADIENT_TYPES.size(); ++i) {
                    const bool isSelected = i == size_t(gradientType);
                    if (ImGui::Selectable(GRADIENT_TYPES[i], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->gradient.has_value()) {
                                values.fill->gradient->type = static_cast<octopus::Gradient::Type>(i);
                            } else {
                                values.fill->gradient = DEFAULT_FILL_GRADIENT;
                            }
                        });
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Color stops
            ImVec4 imColor0 = toImColor(DEFAULT_FILL_GRADIENT_COLOR_0);
            if (octopusFill.gradient.has_value()) {
                imColor0 = toImColor(octopusFill.gradient->stops.front().color);
            }
            ImGui::Text(" Color #0:");
            ImGui::SameLine(100);
            if (ImGui::ColorPicker4(layerPropName(layerId, "shape-fill-gradient-color-0", fillI).c_str(), (float*)&imColor0, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&imColor0, &octopusFill](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    if (!values.fill->gradient.has_value()) {
                        values.fill->gradient = DEFAULT_FILL_GRADIENT;
                    }
                    if (values.fill->gradient->stops.size() < 2) {
                        values.fill->gradient->stops = {
                            DEFAULT_FILL_GRADIENT_COLOR_STOP_0,
                            DEFAULT_FILL_GRADIENT_COLOR_STOP_1
                        };
                    }
                    values.fill->gradient->stops.front().color = toOctopusColor(imColor0);
                });
            }

            ImVec4 imColor1 = toImColor(DEFAULT_FILL_GRADIENT_COLOR_1);
            if (octopusFill.gradient.has_value()) {
                imColor1 = toImColor(octopusFill.gradient->stops.back().color);
            }
            ImGui::Text(" Color #1:");
            ImGui::SameLine(100);
            if (ImGui::ColorPicker4(layerPropName(layerId, "shape-fill-gradient-color-1", fillI).c_str(), (float*)&imColor1, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&imColor1, &octopusFill](octopus::LayerChange::Values &values) {
                    values.fill = octopusFill;
                    if (!values.fill->gradient.has_value()) {
                        values.fill->gradient = DEFAULT_FILL_GRADIENT;
                    }
                    if (values.fill->gradient->stops.size() < 2) {
                        values.fill->gradient->stops = {
                            DEFAULT_FILL_GRADIENT_COLOR_STOP_0,
                            DEFAULT_FILL_GRADIENT_COLOR_STOP_1
                        };
                    }
                    values.fill->gradient->stops.back().color = toOctopusColor(imColor1);
                });
            }
            break;
        }
        case octopus::Fill::Type::IMAGE:
        {
            ImGui::Text("Image:");

            // Open "Open Image File" file dialog
            ImGui::SameLine(100);
            if (ImGui::Button("Load image file")) {
                const char* filters = "Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.*";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseImageFileDlgKey", "Choose an image file", filters, fileDialogContext.imageFilePath, fileDialogContext.imageFileName);
            }

            // Display "Open Octopus File" file dialog
            if (ImGuiFileDialog::Instance()->Display("ChooseImageFileDlgKey")) {
                fileDialogContext.imageFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                fileDialogContext.imageFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

                if (ImGuiFileDialog::Instance()->IsOk()) {
                    const std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                    // Copy the file to images directory
                    std::vector<byte> fileContents;
                    const bool isRead = readFile(filePath, fileContents);
                    if (isRead) {
                        const std::string imageDirectoryStr = (std::string)context.design.imageDirectory;
                        if (!std::filesystem::is_directory(imageDirectoryStr)) {
                            std::filesystem::create_directory(imageDirectoryStr);
                        }
                        const std::string fileName = ((FilePath)filePath).filename();
                        const std::string imageLocation = imageDirectoryStr+"/"+fileName;
                        const bool isCopied = writeFile(imageLocation, fileContents);
                        if (isCopied) {
                            changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, &imageLocation](octopus::LayerChange::Values &values) {
                                values.fill = octopusFill;
                                if (values.fill->image.has_value()) {
                                    values.fill->image->ref.type = octopus::ImageRef::Type::PATH;
                                    values.fill->image->ref.value = imageLocation;
                                } else {
                                    values.fill->image = octopus::Image {
                                        octopus::ImageRef { octopus::ImageRef::Type::PATH, imageLocation },
                                        nonstd::nullopt
                                    };
                                }
                            });
                        }
                    }
                }

                ImGuiFileDialog::Instance()->Close();
            }

            // Image ref value
            ImGui::Text("  Ref type:");
            ImGui::SameLine(100);

            const octopus::ImageRef::Type imageRefType = octopusFill.image.has_value() ? octopusFill.image->ref.type : octopus::ImageRef::Type::PATH;
            if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-image-ref-type", fillI).c_str(), IMAGE_REF_TYPES[imageRefType])) {
                for (size_t i = 0; i < IMAGE_REF_TYPES.size(); ++i) {
                    const bool isSelected = i == size_t(imageRefType);
                    if (ImGui::Selectable(IMAGE_REF_TYPES[i], isSelected)) {
                        changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i](octopus::LayerChange::Values &values) {
                            values.fill = octopusFill;
                            if (values.fill->image.has_value()) {
                                values.fill->image->ref.type = static_cast<octopus::ImageRef::Type>(i);
                            } else {
                                values.fill->image = octopus::Image {
                                    octopus::ImageRef { static_cast<octopus::ImageRef::Type>(i), "" },
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
            char textBuffer[255] {};
            strncpy(textBuffer, imageRefValue.c_str(), sizeof(textBuffer)-1);
            ImGui::Text("  Ref value:");
            ImGui::SameLine(100);
            ImGui::InputText(layerPropName(layerId, "shape-fill-image-ref-value", fillI).c_str(), textBuffer, 255);
            if (ImGui::IsItemEdited()) {
                changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, &textBuffer](octopus::LayerChange::Values &values) {
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

            break;
        }
    }

    if (octopusFill.type == octopus::Fill::Type::GRADIENT || octopusFill.type == octopus::Fill::Type::IMAGE) {
        // Image positioning layout
        ImGui::Text("  Pos. Layout:");
        ImGui::SameLine(100);

        const octopus::Fill::Positioning::Layout positioningLayout = octopusFill.positioning.has_value() ? octopusFill.positioning->layout : octopus::Fill::Positioning::Layout::STRETCH;
        if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-positioning-layout", fillI).c_str(), FILL_POSITIONING_LAYOUTS[positioningLayout])) {
            for (size_t i = 0; i < FILL_POSITIONING_LAYOUTS.size(); ++i) {
                const bool isSelected = i == size_t(positioningLayout);
                if (ImGui::Selectable(FILL_POSITIONING_LAYOUTS[i], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i, &defaultGradientFillPositioningTransform](octopus::LayerChange::Values &values) {
                        values.fill = octopusFill;
                        if (values.fill->positioning.has_value()) {
                            values.fill->positioning->layout = static_cast<octopus::Fill::Positioning::Layout>(i);
                        } else {
                            values.fill->positioning = octopus::Fill::Positioning { static_cast<octopus::Fill::Positioning::Layout>(i), octopus::Fill::Positioning::Origin::LAYER,
                            };
                            memcpy(values.fill->positioning->transform, defaultGradientFillPositioningTransform, sizeof(defaultGradientFillPositioningTransform));
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

        const octopus::Fill::Positioning::Origin positioningOrigin = octopusFill.positioning.has_value() ? octopusFill.positioning->origin : octopus::Fill::Positioning::Origin::LAYER;
        if (ImGui::BeginCombo(layerPropName(layerId, "shape-fill-positioning-origin", fillI).c_str(), FILL_POSITIONING_ORIGINS[positioningOrigin])) {
            for (size_t i = 0; i < FILL_POSITIONING_ORIGINS.size(); ++i) {
                const bool isSelected = i == size_t(positioningOrigin);
                if (ImGui::Selectable(FILL_POSITIONING_ORIGINS[i], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, [&octopusFill, i, &defaultGradientFillPositioningTransform](octopus::LayerChange::Values &values) {
                        values.fill = octopusFill;
                        if (values.fill->positioning.has_value()) {
                            values.fill->positioning->origin = static_cast<octopus::Fill::Positioning::Origin>(i);
                        } else {
                            values.fill->positioning = octopus::Fill::Positioning { octopus::Fill::Positioning::Layout::STRETCH, static_cast<octopus::Fill::Positioning::Origin>(i),
                            };
                            memcpy(values.fill->positioning->transform, defaultGradientFillPositioningTransform, sizeof(defaultGradientFillPositioningTransform));
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
        ImGui::Text("  Pos. transformation:");
        const auto positioningTransform = octopusFill.positioning.has_value() ? octopusFill.positioning->transform : defaultGradientFillPositioningTransform;

        const double a = positioningTransform[0];
        const double b = positioningTransform[1];
        const double c = positioningTransform[2];
        const double d = positioningTransform[3];
        const double trX = positioningTransform[4];
        const double trY = positioningTransform[5];
        const double r = sqrt(a*a+b*b);
        const double s = sqrt(c*c+d*d);

        Vector2f translation { float(trX), float(trY) };
        Vector2f scale { float(r), float(s) };
        float rotation = float((b < 0 ? acos(a/r) : -acos(a/r)) * (180.0/M_PI));
        if (rotation < 0.0f) {
            rotation += 360.0f;
        }

        const auto updateFill = [&octopusFill, &translation, &scale, &rotation](octopus::LayerChange::Values &values) {
            values.fill = octopusFill;
            if (!octopusFill.positioning.has_value()) {
                values.fill->positioning = {
                    octopus::Fill::Positioning::Layout::STRETCH,
                    octopus::Fill::Positioning::Origin::LAYER,
                };
            }
            const double rotationRad = -rotation * (M_PI/180.0);
            values.fill->positioning->transform[0] = scale.x * cos(rotationRad);
            values.fill->positioning->transform[1] = scale.x * sin(rotationRad);
            values.fill->positioning->transform[2] = - scale.y * sin(rotationRad);
            values.fill->positioning->transform[3] = scale.y * cos(rotationRad);
            values.fill->positioning->transform[4] = translation.x;
            values.fill->positioning->transform[5] = translation.y;
        };

        ImGui::Text("    Transl.:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat2(layerPropName(layerId, "fill-translation", fillI).c_str(), &translation.x, 1.0f)) {
            changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, updateFill);
        }

        ImGui::Text("    Scale:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat2(layerPropName(layerId, "fill-scale", fillI).c_str(), &scale.x, 1.0f, 1.0f, 100000.0f)) {
            changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, updateFill);
        }

        ImGui::Text("    Rot.:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat(layerPropName(layerId, "fill-rotation", fillI).c_str(), &rotation, 1.0f, -10000.0f, 10000.0f)) {
            changeReplace(octopus::LayerChange::Subject::FILL, context, component, layerId, fillI, nonstd::nullopt, updateFill);
        }
    }

    // Fill filters
    ImGui::Text("Filters:");
    ImGui::SameLine(415);
    if (ImGui::SmallButton(layerPropName(layerId, "shape-fill-filter-add", fillI, nonstd::nullopt, "+").c_str())) {
        changeInsertBack(octopus::LayerChange::Subject::FILL_FILTER, context, component, layerId, fillI, [](octopus::LayerChange::Values &values) {
            values.filter = DEFAULT_FILL_FILTER;
        });
    }

    if (octopusFill.filters.has_value() && !octopusFill.filters->empty()) {
        const std::vector<octopus::Filter> &octopusFillFilters = *octopusFill.filters;
        int fillFilterToRemove = -1;

        for (int ffI = 0; ffI < static_cast<int>(octopusFill.filters->size()); ffI++) {
            const octopus::Filter &octopusFillFilter = (*octopusFill.filters)[ffI];

            ImGui::Text("  Filter #%i:", ffI);

            ImGui::SameLine(100);
            bool filterVisible = (*octopusFill.filters)[ffI].visible;
            if (ImGui::Checkbox(layerPropName(layerId, "shape-fill-filter-visibility", fillI, ffI).c_str(), &filterVisible)) {
                changeReplace(octopus::LayerChange::Subject::FILL_FILTER, context, component, layerId, fillI, ffI, [octopusFillFilter, filterVisible](octopus::LayerChange::Values &values) {
                    values.filter = octopusFillFilter;
                    values.filter->visible = filterVisible;
                });
            }

            ImGui::SameLine(415);
            if (ImGui::SmallButton(layerPropName(layerId, "shape-fill-filter-remove", fillI, ffI, "-").c_str())) {
                fillFilterToRemove = static_cast<int>(ffI);
            }
        }

        if (fillFilterToRemove >= 0 && fillFilterToRemove < static_cast<int>(octopusFillFilters.size())) {
            changeRemove(octopus::LayerChange::Subject::FILL_FILTER, context, component, layerId, fillI, fillFilterToRemove);
        }
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerShape(const ODE_StringRef &layerId,
                    DesignEditorContext &context,
                    DesignEditorComponent &component,
                    DesignEditorUIState::FileDialog &fileDialogContext,
                    const octopus::Shape &octopusShape,
                    const ODE_LayerMetrics &layerMetrics) {
    // Rounded corner radius (for rectangles)
    const bool isRectangle = (octopusShape.path.has_value() && octopusShape.path->type == octopus::Path::Type::RECTANGLE);
    if (isRectangle) {
        const octopus::Path &octopusShapePath = *octopusShape.path;
        float cornerRadius = float(octopusShape.path->cornerRadius.has_value() ? *octopusShape.path->cornerRadius : 0.0);
        ImGui::Text("Corner radius:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat(layerPropName(layerId, "shape-rectangle-corner-radius").c_str(), &cornerRadius, 1.0f, 0.0f, 1000.0f)) {
            changeProperty(octopus::LayerChange::Subject::SHAPE, context, component, layerId, nonstd::nullopt, [&cornerRadius, &octopusShapePath](octopus::LayerChange::Values &values) {
                values.path = octopusShapePath;
                values.path->cornerRadius = cornerRadius;
            });
        }

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
    }

    // Shape strokes
    const std::vector<octopus::Shape::Stroke> &octopusShapeStrokes = octopusShape.strokes;
    if (ImGui::CollapsingHeader(layerHeaderLabel("Shape strokes", layerId).c_str())) {
        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
        ImGui::SameLine(415);
        if (ImGui::SmallButton(layerPropName(layerId, "shape-stroke-add", nonstd::nullopt, nonstd::nullopt, "+").c_str())) {
            changeInsertBack(octopus::LayerChange::Subject::STROKE, context, component, layerId, nonstd::nullopt, [](octopus::LayerChange::Values &values) {
                values.stroke = DEFAULT_SHAPE_STROKE;
            });
        }
        int strokeToRemove = -1;
        for (size_t si = 0; si < octopusShapeStrokes.size(); ++si) {
            ImGui::Text("Stroke #%i:", static_cast<int>(si));

            ImGui::SameLine(415);
            if (ImGui::SmallButton(layerPropName(layerId, "shape-stroke-remove", si, nonstd::nullopt, "-").c_str())) {
                strokeToRemove = static_cast<int>(si);
            }

            drawLayerShapeStroke(static_cast<int>(si), layerId, context, component, octopusShapeStrokes[si]);
        }
        if (strokeToRemove >= 0 && strokeToRemove < static_cast<int>(octopusShapeStrokes.size())) {
            changeRemove(octopus::LayerChange::Subject::STROKE, context, component, layerId, strokeToRemove, nonstd::nullopt);
        }
    }

    // Shape Fills
    const std::vector<octopus::Fill> &octopusShapeFills = octopusShape.fills;
    if (ImGui::CollapsingHeader(layerHeaderLabel("Shape fills", layerId).c_str())) {
        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
        ImGui::SameLine(415);
        if (ImGui::SmallButton(layerPropName(layerId, "shape-fill-add", nonstd::nullopt, nonstd::nullopt, "+").c_str())) {
            changeInsertBack(octopus::LayerChange::Subject::FILL, context, component, layerId, nonstd::nullopt, [](octopus::LayerChange::Values &values) {
                values.fill = DEFAULT_FILL;
            });
        }
        int fillToRemove = -1;
        for (size_t fi = 0; fi < octopusShapeFills.size(); ++fi) {
            ImGui::Text("Fill #%i:", static_cast<int>(fi));

            ImGui::SameLine(415);
            if (ImGui::SmallButton(layerPropName(layerId, "shape-fill-remove", fi, nonstd::nullopt, "-").c_str())) {
                fillToRemove = static_cast<int>(fi);
            }

            drawLayerShapeFill(static_cast<int>(fi), layerId, context, component, fileDialogContext, octopusShape.fills[fi], layerMetrics);
        }
        if (fillToRemove >= 0 && fillToRemove < static_cast<int>(octopusShape.fills.size())) {
            changeRemove(octopus::LayerChange::Subject::FILL, context, component, layerId, fillToRemove, nonstd::nullopt);
        }
    }
}

void drawLayerEffects(const ODE_StringRef &layerId,
                      DesignEditorContext &context,
                      DesignEditorComponent &component,
                      const std::vector<octopus::Effect> &octopusEffects) {
    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
    ImGui::SameLine(415);
    if (ImGui::SmallButton(layerPropName(layerId, "effect-add", nonstd::nullopt, nonstd::nullopt, "+").c_str())) {
        changeInsertBack(octopus::LayerChange::Subject::EFFECT, context, component, layerId, nonstd::nullopt, [](octopus::LayerChange::Values &values) {
            values.effect = DEFAULT_EFFECT;
        });
    }
    int effectToRemove = -1;
    for (size_t ei = 0; ei < octopusEffects.size(); ++ei) {
        const octopus::Effect &octopusEffect = octopusEffects[ei];

        ImGui::Text("Effect #%i:", static_cast<int>(ei));

        ImGui::SameLine(415);
        if (ImGui::SmallButton(layerPropName(layerId, "effect-remove", ei, nonstd::nullopt, "-").c_str())) {
            effectToRemove = static_cast<int>(ei);
        }

        // Visibility
        ImGui::Text("Visible:");
        ImGui::SameLine(100);
        bool effectVisible = octopusEffect.visible;
        if (ImGui::Checkbox(layerPropName(layerId, "effect-visibility", ei).c_str(), &effectVisible)) {
            changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, effectVisible](octopus::LayerChange::Values &values) {
                values.effect = octopusEffect;
                values.effect->visible = effectVisible;
            });
        }

        ImGui::Text("Blend mode:");
        ImGui::SameLine(100);
        if (ImGui::BeginCombo(layerPropName(layerId, "effect-blend-mode", ei).c_str(), BLEND_MODES[octopusEffect.blendMode])) {
            for (size_t i = 0; i < BLEND_MODES.size(); ++i) {
                const bool isSelected = i == size_t(octopusEffect.blendMode);
                if (ImGui::Selectable(BLEND_MODES[i], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, i](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->blendMode = static_cast<octopus::BlendMode>(i);
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
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, ebI](octopus::LayerChange::Values &values) {
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
        if (ImGui::BeginCombo(layerPropName(layerId, "effect-type", ei).c_str(), EFFECT_TYPES[octopusEffect.type])) {
            for (size_t i = 0; i < EFFECT_TYPES.size(); ++i) {
                const bool isSelected = i == size_t(octopusEffect.type);
                if (ImGui::Selectable(EFFECT_TYPES[i], isSelected)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, i](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->type = static_cast<octopus::Effect::Type>(i);
                        switch (values.effect->type) {
                            case octopus::Effect::Type::OVERLAY:
                                if (!octopusEffect.overlay.has_value()) {
                                    values.effect->overlay = DEFAULT_FILL;
                                }
                                break;
                            case octopus::Effect::Type::STROKE:
                                if (!octopusEffect.stroke.has_value()) {
                                    values.effect->stroke = DEFAULT_STROKE;
                                }
                                break;
                            case octopus::Effect::Type::DROP_SHADOW:
                            case octopus::Effect::Type::INNER_SHADOW:
                                if (!octopusEffect.shadow.has_value()) {
                                    values.effect->shadow = DEFAULT_EFFECT_SHADOW;
                                }
                                break;
                            case octopus::Effect::Type::OUTER_GLOW:
                            case octopus::Effect::Type::INNER_GLOW:
                                if (!octopusEffect.glow.has_value()) {
                                    values.effect->glow = DEFAULT_EFFECT_GLOW;
                                }
                                break;
                            case octopus::Effect::Type::GAUSSIAN_BLUR:
                            case octopus::Effect::Type::BOUNDED_BLUR:
                            case octopus::Effect::Type::BLUR:
                                if (!octopusEffect.blur.has_value()) {
                                    values.effect->blur = DEFAULT_EFFECT_BLUR;
                                }
                                break;
                            case octopus::Effect::Type::OTHER:
                                break;
                        }
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
                const octopus::Fill &octopusEffectOverlay = octopusEffect.overlay.has_value() ? *octopusEffect.overlay : DEFAULT_FILL;
                if (octopusEffectOverlay.type != octopus::Fill::Type::COLOR) {
                    ImGui::Text("Unsupported overlay effect type.");
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
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->overlay->type = octopus::Fill::Type::COLOR;
                        values.effect->overlay->color = toOctopusColor(imColor);
                    });
                }

                // Effect fill filters (overlay)
                ImGui::Text("Overlay filters:");
                ImGui::SameLine(415);
                if (ImGui::SmallButton(layerPropName(layerId, "effect-overlay-fill-filter-add", ei, nonstd::nullopt, "+").c_str())) {
                    changeInsertBack(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, [](octopus::LayerChange::Values &values) {
                        values.filter = DEFAULT_FILL_FILTER;
                    });
                }

                if (octopusEffectOverlay.filters.has_value() && !octopusEffectOverlay.filters->empty()) {
                    int effectFillFilterToRemove = -1;

                    for (int effI = 0; effI < static_cast<int>(octopusEffectOverlay.filters->size()); effI++) {
                        ImGui::Text("  Filter #%i:", effI);
                        ImGui::SameLine(100);

                        const octopus::Filter &octopusEffectOverlayFilter = (*octopusEffectOverlay.filters)[effI];

                        bool effectFillFilterVisible = octopusEffectOverlayFilter.visible;
                        if (ImGui::Checkbox(layerPropName(layerId, "effect-overlay-fill-filter-visibility", ei, effI).c_str(), &effectFillFilterVisible)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, effI, [&octopusEffectOverlayFilter, effectFillFilterVisible](octopus::LayerChange::Values &values) {
                                values.filter = octopusEffectOverlayFilter;
                                values.filter->visible = effectFillFilterVisible;
                            });
                        }

                        ImGui::SameLine(415);
                        if (ImGui::SmallButton(layerPropName(layerId, "effect-overlay-fill-filter-remove", ei, effI, "-").c_str())) {
                            effectFillFilterToRemove = static_cast<int>(effI);
                        }
                    }

                    if (effectFillFilterToRemove >= 0 && effectFillFilterToRemove < static_cast<int>(octopusEffectOverlay.filters->size())) {
                        changeRemove(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, effectFillFilterToRemove);
                    }
                }
                break;
            }
            case octopus::Effect::Type::STROKE:
            {
                const octopus::Stroke &octopusEffectStroke = octopusEffect.stroke.has_value() ? *octopusEffect.stroke : DEFAULT_STROKE;
                const octopus::Fill &octopusEffectStrokeFill = octopusEffectStroke.fill;

                // Thickness
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float strokeThickness = float(octopusEffectStroke.thickness);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-stroke-thickness", ei).c_str(), &strokeThickness, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&strokeThickness, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->stroke->thickness = strokeThickness;
                    });
                }

                // Position
                ImGui::Text("Position:");
                ImGui::SameLine(100);
                if (ImGui::BeginCombo(layerPropName(layerId, "effect-stroke-position", ei).c_str(), STROKE_POSITIONS[octopusEffectStroke.position])) {
                    for (size_t i = 0; i < STROKE_POSITIONS.size(); ++i) {
                        const bool isSelected = i == size_t(octopusEffectStroke.position);
                        if (ImGui::Selectable(STROKE_POSITIONS[i], isSelected)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei,nonstd::nullopt, [i, &octopusEffect](octopus::LayerChange::Values &values) {
                                values.effect = octopusEffect;
                                values.effect->stroke->position = static_cast<octopus::Stroke::Position>(i);
                            });
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                // Color
                ImVec4 imColor = toImColor(DEFAULT_STROKE_FILL_COLOR);
                if (octopusEffectStrokeFill.type == octopus::Fill::Type::COLOR && octopusEffectStrokeFill.color.has_value()) {
                    imColor = toImColor(*octopusEffectStrokeFill.color);
                }
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-stroke-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->stroke->fill.type = octopus::Fill::Type::COLOR;
                        values.effect->stroke->fill.color = toOctopusColor(imColor);
                    });
                }

                // Effect fill filters (stroke)
                ImGui::Text("Stroke filters:");
                ImGui::SameLine(415);
                if (ImGui::SmallButton(layerPropName(layerId, "effect-stroke-fill-filter-add", ei, nonstd::nullopt, "+").c_str())) {
                    changeInsertBack(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, [](octopus::LayerChange::Values &values) {
                        values.filter = DEFAULT_FILL_FILTER;
                    });
                }

                if (octopusEffectStroke.fill.filters.has_value() && !octopusEffectStroke.fill.filters->empty()) {
                    int effectFillFilterToRemove = -1;

                    for (int effI = 0; effI < static_cast<int>(octopusEffectStroke.fill.filters->size()); effI++) {
                        ImGui::Text("  Filter #%i:", effI);
                        ImGui::SameLine(100);

                        const octopus::Filter &octopusEffectStrokeFillFilter = (*octopusEffectStroke.fill.filters)[effI];

                        bool effectFillFilterVisible = octopusEffectStrokeFillFilter.visible;
                        if (ImGui::Checkbox(layerPropName(layerId, "effect-stroke-fill-filter-visibility", ei, effI).c_str(), &effectFillFilterVisible)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, effI, [&octopusEffectStrokeFillFilter, effectFillFilterVisible](octopus::LayerChange::Values &values) {
                                values.filter = octopusEffectStrokeFillFilter;
                                values.filter->visible = effectFillFilterVisible;
                            });
                        }

                        ImGui::SameLine(415);
                        if (ImGui::SmallButton(layerPropName(layerId, "effect-stroke-fill-filter-remove", ei, effI, "-").c_str())) {
                            effectFillFilterToRemove = static_cast<int>(effI);
                        }
                    }

                    if (effectFillFilterToRemove >= 0 && effectFillFilterToRemove < static_cast<int>(octopusEffectStroke.fill.filters->size())) {
                        changeRemove(octopus::LayerChange::Subject::EFFECT_FILL_FILTER, context, component, layerId, ei, effectFillFilterToRemove);
                    }
                }
                break;
            }
            case octopus::Effect::Type::DROP_SHADOW:
            case octopus::Effect::Type::INNER_SHADOW:
            {
                const octopus::Shadow &octopusEffectShadow = octopusEffect.shadow.has_value() ? *octopusEffect.shadow : DEFAULT_EFFECT_SHADOW;

                // Effect shadow offset
                ImGui::Text("Offset:");
                ImGui::SameLine(100);
                Vector2f shadowOffset { static_cast<float>(octopusEffectShadow.offset.x), static_cast<float>(octopusEffectShadow.offset.y) };
                if (ImGui::DragFloat2(layerPropName(layerId, "effect-shadow-offset", ei).c_str(), &shadowOffset.x, 0.05f, -1000.0f, 1000.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, &shadowOffset](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->offset.x = shadowOffset.x;
                        values.effect->shadow->offset.y = shadowOffset.y;
                    });
                }

                // Effect shadow blur
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float shadowBlur = float(octopusEffectShadow.blur);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-shadow-blur", ei).c_str(), &shadowBlur, 0.1f, -1000.0f, 1000.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&shadowBlur, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->blur = shadowBlur;
                    });
                }

                // Effect shadow choke
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float shadowChoke = float(octopusEffectShadow.choke);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-shadow-choke", ei).c_str(), &shadowChoke, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&shadowChoke, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->choke = shadowChoke;
                    });
                }

                // Effect shadow color
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                const ImVec4 &imColor = toImColor(octopusEffectShadow.color);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-shadow-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->shadow->color = toOctopusColor(imColor);
                    });
                }
                break;
            }
            case octopus::Effect::Type::OUTER_GLOW:
            case octopus::Effect::Type::INNER_GLOW:
            {
                const octopus::Shadow &octopusEffectGlow = octopusEffect.glow.has_value() ? *octopusEffect.glow : DEFAULT_EFFECT_GLOW;

                // Effect shadow offset
                ImGui::Text("Offset:");
                ImGui::SameLine(100);
                Vector2f shadowOffset { static_cast<float>(octopusEffectGlow.offset.x), static_cast<float>(octopusEffectGlow.offset.y) };
                if (ImGui::DragFloat2(layerPropName(layerId, "effect-glow-offset", ei).c_str(), &shadowOffset.x, 0.05f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, &shadowOffset](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->offset.x = shadowOffset.x;
                        values.effect->glow->offset.y = shadowOffset.y;
                    });
                }

                // Effect shadow blur
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float shadowBlur = float(octopusEffectGlow.blur);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-glow-blur", ei).c_str(), &shadowBlur, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&shadowBlur, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->blur = shadowBlur;
                    });
                }

                // Effect shadow choke
                ImGui::Text("Thickess:");
                ImGui::SameLine(100);
                float shadowChoke = float(octopusEffectGlow.choke);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-glow-choke", ei).c_str(), &shadowChoke, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&shadowChoke, &octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->glow->choke = shadowChoke;
                    });
                }

                // Effect shadow color
                ImGui::Text("Color:");
                ImGui::SameLine(100);
                const ImVec4 &imColor = toImColor(octopusEffectGlow.color);
                if (ImGui::ColorPicker4(layerPropName(layerId, "effect-glow-color", ei).c_str(), (float*)&imColor, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&imColor, &octopusEffect](octopus::LayerChange::Values &values) {
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
                ImGui::Text("Blur:");
                ImGui::SameLine(100);
                float blurAmount = float(octopusEffect.blur.has_value() ? *octopusEffect.blur : DEFAULT_EFFECT_BLUR);
                if (ImGui::DragFloat(layerPropName(layerId, "effect-blur-amount", ei).c_str(), &blurAmount, 0.1f, 0.0f, 100.0f)) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, blurAmount](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        values.effect->blur = blurAmount;
                    });
                }

                // Effect filters (only for blurs)
                ImGui::Text("Filters:");
                ImGui::SameLine(415);
                if (ImGui::SmallButton(layerPropName(layerId, "effect-filter-add", ei, nonstd::nullopt, "+").c_str())) {
                    changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect](octopus::LayerChange::Values &values) {
                        values.effect = octopusEffect;
                        if (values.effect->filters.has_value()) {
                            values.effect->filters->emplace_back(DEFAULT_FILL_FILTER);
                        } else {
                            values.effect->filters = { DEFAULT_FILL_FILTER };
                        }
                    });
                }

                if (octopusEffect.filters.has_value() && !octopusEffect.filters->empty()) {
                    int effectFilterToRemove = -1;

                    for (int efI = 0; efI < static_cast<int>(octopusEffect.filters->size()); efI++) {
                        ImGui::Text("  Filter #%i:", efI);
                        ImGui::SameLine(100);

                        bool effectFilterVisible = (*octopusEffect.filters)[efI].visible;
                        if (ImGui::Checkbox(layerPropName(layerId, "effect-filter-visibility", ei, efI).c_str(), &effectFilterVisible)) {
                            changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, efI, effectFilterVisible](octopus::LayerChange::Values &values) {
                                values.effect = octopusEffect;
                                (*values.effect->filters)[efI].visible = effectFilterVisible;
                            });
                        }

                        ImGui::SameLine(415);
                        if (ImGui::SmallButton(layerPropName(layerId, "effect-filter-remove", ei, efI, "-").c_str())) {
                            effectFilterToRemove = static_cast<int>(efI);
                        }
                    }

                    if (effectFilterToRemove >= 0 && effectFilterToRemove < static_cast<int>(octopusEffect.filters->size())) {
                        changeReplace(octopus::LayerChange::Subject::EFFECT, context, component, layerId, ei, nonstd::nullopt, [&octopusEffect, effectFilterToRemove](octopus::LayerChange::Values &values) {
                            values.effect = octopusEffect;
                            values.effect->filters->erase(values.effect->filters->begin()+effectFilterToRemove);
                        });
                    }
                }
                break;
            }
            case octopus::Effect::Type::OTHER:
            {
                break;
            }
        }
    }
    if (effectToRemove >= 0 && effectToRemove < static_cast<int>(octopusEffects.size())) {
        changeRemove(octopus::LayerChange::Subject::EFFECT, context, component, layerId, effectToRemove, nonstd::nullopt);
    }

    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
}

void drawLayerPropertiesWidget(DesignEditorContext &context,
                               DesignEditorComponent &component,
                               DesignEditorUIState::LayerSelection &layerSelection,
                               DesignEditorUIState::FileDialog &fileDialogContext) {
    ImGui::SetNextWindowSize(ImVec2(450, 870), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(1120, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Selected Layer Properties");

    if (component.component.ptr == nullptr) {
        ImGui::End();
        return;
    }

    ODE_String octopusString;
    ode_component_getOctopus(component.component, &octopusString);

    octopus::Octopus componentOctopus;
    octopus::Parser::parse(componentOctopus, octopusString.data);

    if (!componentOctopus.content.has_value()) {
        ImGui::End();
        return;
    }

    ODE_LayerList &layerList = component.layerList;

    for (int i = 0; i < layerList.n; ++i) {
        const ODE_LayerList::Entry &layer = layerList.entries[i];
        if (layerSelection.isSelected(layer.id.data)) {
            const octopus::Layer *octopusLayerPtr = findLayer(*componentOctopus.content, ode_stringDeref(layer.id));
            if (octopusLayerPtr == nullptr) {
                continue;
            }
            const octopus::Layer &octopusLayer = *octopusLayerPtr;

            ODE_LayerMetrics layerMetrics;
            if (ode_component_getLayerMetrics(component.component, layer.id, &layerMetrics) != ODE_RESULT_OK) {
                continue;
            }

            const std::string layerSectionHeader =
                std::string("[")+layerTypeToShortString(layer.type)+std::string("] ")+
                ode_stringDeref(layer.id)+std::string(" ")+
                std::string("(")+ode_stringDeref(layer.name)+std::string(")");

            if (ImGui::CollapsingHeader(layerSectionHeader.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                // Layer info
                drawLayerInfo(layer, context, component);

                // Common layer properties
                drawLayerCommonProperties(layer.id, octopusLayer, context, component);

                // Transformation
                drawLayerTransformation(layer.id, context, component, layerMetrics);

                // Text
                const bool isTextLayer = (layer.type == ODE_LAYER_TYPE_TEXT && octopusLayer.type == octopus::Layer::Type::TEXT && octopusLayer.text.has_value());
                if (isTextLayer && ImGui::CollapsingHeader(layerHeaderLabel("Text", layer.id).c_str())) {
                    drawLayerText(layer.id, context, component, *octopusLayer.text);
                }

                // Shape
                const bool isShapeLayer = (layer.type == ODE_LAYER_TYPE_SHAPE && octopusLayer.type == octopus::Layer::Type::SHAPE && octopusLayer.shape.has_value());
                if (isShapeLayer) {
                    drawLayerShape(layer.id, context, component, fileDialogContext, *octopusLayer.shape, layerMetrics);
                }

                // Effects
                if (ImGui::CollapsingHeader(layerHeaderLabel("Effects", layer.id).c_str())) {
                    drawLayerEffects(layer.id, context, component, octopusLayer.effects);
                }

                const octopus::Layer *octopusParentLayerPtr = findLayer(*componentOctopus.content, ode_stringDeref(layer.parentId));
                const bool isMask = octopusParentLayerPtr != nullptr && octopusParentLayerPtr->type == octopus::Layer::Type::MASK_GROUP && octopusParentLayerPtr->mask.has_value() && octopusParentLayerPtr->mask->id == octopusLayer.id;

                if (!isMask) {
                    ImGui::Dummy(ImVec2 { 0.0f, 10.0f });
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_DARK_RED);
                    ImGui::SameLine(100);
                    if (ImGui::Button(layerPropName(layer.id, "delete", nonstd::nullopt, nonstd::nullopt, "Delete Layer").c_str(), ImVec2 { 250, 20 })) {
                        if (ode_component_removeLayer(component.component, layer.id) == ODE_RESULT_OK) {
                            layerSelection.remove(layer.id.data);
                            ode_component_listLayers(component.component, &layerList);
                            ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
                        }
                    }
                    ImGui::PopStyleColor(1);
                }

                ImGui::Dummy(ImVec2 { 0.0f, 20.0f });
            }
        }
    }

    ImGui::End();
}
