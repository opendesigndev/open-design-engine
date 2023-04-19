
#include "DesignEditorDesignViewWidget.h"

#include <imgui.h>

#include <ode/renderer-api.h>
#include <ode-essentials.h>
#include <ode-logic.h>
#include <ode-renderer.h>

#include "DesignEditorUIValues.h"

using namespace ode;

namespace {

void drawImGuiWidgetTexture(GLuint textureHandle,
                            int width, int height,
                            float &zoom,
                            std::optional<ImVec2> &mouseClickPos,
                            std::optional<ImVec2> &mouseDragPos,
                            size_t colsCount = 1, size_t rowsCount = 1) {
    const ImVec2 windowSize = ImGui::GetWindowSize();

    const int horizontalPadding = 18;
    const int verticalPadding = 100;
    const float scaling = std::min(
        static_cast<float>(windowSize.x / static_cast<float>(colsCount) - horizontalPadding) / static_cast<float>(width),
        static_cast<float>(windowSize.y / static_cast<float>(rowsCount) - verticalPadding) / static_cast<float>(height));

    const ImVec2 newImageSize(std::max(scaling * width, 0.0f) * zoom, std::max(scaling * height, 0.0f) * zoom);

    ImGui::Text("GL Handle:        %d", textureHandle);
    ImGui::Text("Texture size:     %d x %d", width, height);
    ImGui::Text("Display size:     %d x %d", static_cast<int>(std::round(newImageSize.x)), static_cast<int>(std::round(newImageSize.y)));
    ImGui::SliderFloat("Zoom [-S][+W]", &zoom, 1.0f, 10.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COLOR_LIGHT_BLUE);
    ImGui::ImageButton((void*)(intptr_t)textureHandle, newImageSize);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        mouseClickPos = ImGui::GetMousePos();
    } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        mouseClickPos = std::nullopt;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        mouseDragPos = ImGui::GetMousePos();
    } else {
        mouseDragPos = std::nullopt;
    }

    ImGui::PopStyleColor(2);
}

}

void drawDesignViewWidget(const ODE_ComponentHandle &component,
                          const ODE_Bitmap &bitmap,
                          DesignEditorRenderer &renderer,
                          DesignEditorUIState::Textures &texturesContext,
                          DesignEditorUIState::Canvas &canvasContext,
                          const DesignEditorUIState::LayerSelection &layerSelection,
                          const ODE_StringRef &topLayerId,
                          int selectedDisplayMode) {
    ImGui::Begin("Design View");

    if (bitmap.width > 0 && bitmap.height > 0) {
        ode::Bitmap bmp(PixelFormat::PREMULTIPLIED_RGBA, reinterpret_cast<const void*>(bitmap.pixels), bitmap.width, bitmap.height);

        const ScaledBounds placement {0,0,static_cast<double>(bmp.width()),static_cast<double>(bmp.height())};

        const auto toCanvasSpace = [&canvasContext](const ImVec2 &posInScreenSpace)->ImVec2 {
            return ImVec2 {
                (posInScreenSpace.x - canvasContext.bbMin.x) / canvasContext.bbSize.x,
                (posInScreenSpace.y - canvasContext.bbMin.y) / canvasContext.bbSize.y,
            };
        };

        AnnotationRectangleOpt selectionRectangle = std::nullopt;
        if (canvasContext.mouseClickPos.has_value() &&
            canvasContext.mouseDragPos.has_value()) {
            const ImVec2 rectStart = toCanvasSpace(*canvasContext.mouseClickPos);
            const ImVec2 rectEnd = toCanvasSpace(*canvasContext.mouseDragPos);

            selectionRectangle = ode::Rectangle<float> {
                std::min(rectStart.x, rectEnd.x),
                std::min(rectStart.y, rectEnd.y),
                std::max(rectStart.x, rectEnd.x),
                std::max(rectStart.y, rectEnd.y) };
        }

        AnnotationRectangles highlightRectangles;
        for (const ODE_StringRef &layerId : layerSelection.layerIDs) {
            ODE_LayerMetrics topLayerMetrics;
            ode_component_getLayerMetrics(component, topLayerId, &topLayerMetrics);

            const ODE_Rectangle &topLayerBounds = topLayerMetrics.logicalBounds;
            const float canvasWidth = topLayerBounds.b.x - topLayerBounds.a.x;
            const float canvasHeight = topLayerBounds.b.y - topLayerBounds.a.y;

            ODE_LayerMetrics layerMetrics;
            ode_component_getLayerMetrics(component, layerId, &layerMetrics);
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

            std::vector<Vector2<float>> corners {
                { static_cast<float>(layerBounds.a.x), static_cast<float>(layerBounds.a.y) },
                { static_cast<float>(layerBounds.b.x), static_cast<float>(layerBounds.a.y) },
                { static_cast<float>(layerBounds.b.x), static_cast<float>(layerBounds.b.y) },
                { static_cast<float>(layerBounds.a.x), static_cast<float>(layerBounds.b.y) },
            };
            std::transform(corners.begin()+1, corners.end(), corners.begin()+1, [&rotation, &center=corners.front()](Vector2<float> p) {
                p -= center;

                const float s = sin(rotation);
                const float c = cos(rotation);

                return Vector2<float>{
                    p.x*c - p.y*s + center.x,
                    p.x*s + p.y*c + center.y,
                };
            });

            highlightRectangles.emplace_back(ode::Rectangle<float> {
                std::clamp(std::min(std::min(corners[0].x, corners[1].x), std::min(corners[2].x, corners[3].x)) / canvasWidth, 0.0f, 1.0f),
                std::clamp(std::min(std::min(corners[0].y, corners[1].y), std::min(corners[2].y, corners[3].y)) / canvasHeight, 0.0f, 1.0f),
                std::clamp(std::max(std::max(corners[0].x, corners[1].x), std::max(corners[2].x, corners[3].x)) / canvasWidth, 0.0f, 1.0f),
                std::clamp(std::max(std::max(corners[0].y, corners[1].y), std::max(corners[2].y, corners[3].y)) / canvasHeight, 0.0f, 1.0f),
            });
        }

        texturesContext.designImageTexture = renderer.blendImageToTexture(std::move(bmp), placement, selectedDisplayMode, selectionRectangle, highlightRectangles);

        drawImGuiWidgetTexture(texturesContext.designImageTexture->getInternalGLHandle(),
                               texturesContext.designImageTexture->dimensions().x,
                               texturesContext.designImageTexture->dimensions().y,
                               canvasContext.zoom,
                               canvasContext.mouseClickPos,
                               canvasContext.mouseDragPos);

        canvasContext.isMouseOver = ImGui::IsItemHovered();
        canvasContext.bbSize = ImGui::GetItemRectSize();
        canvasContext.bbMin = ImGui::GetItemRectMin();
        canvasContext.bbMax = ImGui::GetItemRectMax();
    }

    ImGui::End();
}
