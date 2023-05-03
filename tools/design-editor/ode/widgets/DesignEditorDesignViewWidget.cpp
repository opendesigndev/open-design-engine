
#include "DesignEditorDesignViewWidget.h"

#include <imgui.h>

#include <ode/renderer-api.h>
#include <ode-essentials.h>
#include <ode-logic.h>
#include <ode-renderer.h>

#include "DesignEditorUIValues.h"
#include "DesignEditorUIHelpers.h"

using namespace ode;

namespace {

void drawImGuiWidgetTexture(GLuint textureHandle,
                            int width, int height,
                            float &zoom,
                            std::optional<ImVec2> &mouseClickPos,
                            std::optional<ImVec2> &mouseDragPos,
                            std::optional<ImVec2> &prevMouseDragPos,
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
        prevMouseDragPos = mouseDragPos;
        mouseDragPos = ImGui::GetMousePos();
    } else {
        prevMouseDragPos = std::nullopt;
        mouseDragPos = std::nullopt;
    }

    ImGui::PopStyleColor(2);
}

}

void drawDesignViewWidget(const ODE_ComponentHandle &component,
                          const ODE_Bitmap &bitmap,
                          DesignEditorRenderer &renderer,
                          DesignEditorUIState::Mode uiMode,
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
        if (uiMode == DesignEditorUIState::Mode::SELECT &&
            canvasContext.mouseClickPos.has_value() &&
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

            highlightRectangles.emplace_back(ode::Rectangle<float> {
                std::clamp(static_cast<float>(layerMetrics.transformedGraphicalBounds.a.x) / canvasWidth, 0.0f, 1.0f),
                std::clamp(static_cast<float>(layerMetrics.transformedGraphicalBounds.a.y) / canvasHeight, 0.0f, 1.0f),
                std::clamp(static_cast<float>(layerMetrics.transformedGraphicalBounds.b.x) / canvasWidth, 0.0f, 1.0f),
                std::clamp(static_cast<float>(layerMetrics.transformedGraphicalBounds.b.y) / canvasHeight, 0.0f, 1.0f),
            });
        }

        texturesContext.designImageTexture = renderer.blendImageToTexture(std::move(bmp), placement, selectedDisplayMode, selectionRectangle, highlightRectangles);

        drawImGuiWidgetTexture(texturesContext.designImageTexture->getInternalGLHandle(),
                               texturesContext.designImageTexture->dimensions().x,
                               texturesContext.designImageTexture->dimensions().y,
                               canvasContext.zoom,
                               canvasContext.mouseClickPos,
                               canvasContext.mouseDragPos,
                               canvasContext.prevMouseDragPos);

        canvasContext.isMouseOver = ImGui::IsItemHovered();
        canvasContext.bbSize = ImGui::GetItemRectSize();
        canvasContext.bbMin = ImGui::GetItemRectMin();
        canvasContext.bbMax = ImGui::GetItemRectMax();
    }

    ImGui::End();
}
