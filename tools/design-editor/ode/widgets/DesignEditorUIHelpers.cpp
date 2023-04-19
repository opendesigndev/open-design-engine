
#include "DesignEditorUIHelpers.h"

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

std::optional<std::string> findParentLayerId(const ODE_LayerList &layerList, const ODE_StringRef &layerId) {
    for (int i = 0; i < layerList.n; ++i) {
        const ODE_LayerList::Entry &layer = layerList.entries[i];
        if (strcmp(layerId.data, layer.id.data) == 0) {
            return ode_stringDeref(layer.parentId);
        }
    }
    return std::nullopt;
}

std::optional<std::string> findAvailableLayerId(const std::string &prefix, const ODE_LayerList &layerList) {
    for (int i = 0; i < std::numeric_limits<int>::max(); ++i) {
        const std::string layerId = prefix + "_" + std::to_string(i);
        bool idAlreadyInUse = false;
        for (int j = 0; j < layerList.n && !idAlreadyInUse; ++j) {
            if (layerId == ode_stringDeref(layerList.entries[j].id)) {
                idAlreadyInUse = true;
            }
        }
        if (!idAlreadyInUse) {
            return layerId;
        }
    }
    return std::nullopt;
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

bool isImGuiMultiselectKeyModifierPressed() {
    return
        ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
        ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) ||
        ImGui::IsKeyDown(ImGuiKey_RightShift) || ImGui::IsKeyDown(ImGuiKey_RightSuper);
}

bool isRectangleIntersection(const ODE_Rectangle &r1, const ODE_Rectangle &r2) {
    return (!(r1.b.x < r2.a.x || r2.b.x < r1.a.x || r1.b.y < r2.a.y || r2.b.y < r1.a.y));
}

ODE_Rectangle getBoundingRectangle(const ODE_LayerMetrics &layerMetrics) {
    ODE_Rectangle layerBounds = layerMetrics.graphicalBounds;

    const float w = layerBounds.b.x - layerBounds.a.x;
    const float h = layerBounds.b.y - layerBounds.a.y;

    const float a = static_cast<float>(layerMetrics.transformation.matrix[0]);
    const float b = static_cast<float>(layerMetrics.transformation.matrix[1]);
    const float c = static_cast<float>(layerMetrics.transformation.matrix[2]);
    const float d = static_cast<float>(layerMetrics.transformation.matrix[3]);
    const float trX = static_cast<float>(layerMetrics.transformation.matrix[4]);
    const float trY = static_cast<float>(layerMetrics.transformation.matrix[5]);
    const float sX = sqrt(a*a+b*b);
    const float sY = sqrt(c*c+d*d);
    const float rotation = (b < 0) ? -acos(a/sX) : acos(a/sX);

    layerBounds.a.x += trX;
    layerBounds.a.y += trY;
    layerBounds.b.x += trX + w*(sX-1.0);
    layerBounds.b.y += trY + h*(sY-1.0);

    std::vector<ODE_Vector2> corners {
        { layerBounds.a.x, layerBounds.a.y },
        { layerBounds.b.x, layerBounds.a.y },
        { layerBounds.b.x, layerBounds.b.y },
        { layerBounds.a.x, layerBounds.b.y },
    };
    std::transform(corners.begin()+1, corners.end(), corners.begin()+1, [&rotation, &center=corners.front()](ODE_Vector2 p) {
        p.x -= center.x;
        p.y -= center.y;

        const float s = sin(rotation);
        const float c = cos(rotation);

        return ODE_Vector2 {
            p.x*c - p.y*s + center.x,
            p.x*s + p.y*c + center.y,
        };
    });

    layerBounds.a.x = std::min(std::min(corners[0].x, corners[1].x), std::min(corners[2].x, corners[3].x));
    layerBounds.a.y = std::min(std::min(corners[0].y, corners[1].y), std::min(corners[2].y, corners[3].y));
    layerBounds.b.x = std::max(std::max(corners[0].x, corners[1].x), std::max(corners[2].x, corners[3].x));
    layerBounds.b.y = std::max(std::max(corners[0].y, corners[1].y), std::max(corners[2].y, corners[3].y));

    return layerBounds;
}
