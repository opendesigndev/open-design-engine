
#include "DesignEditorDesignViewWidget.h"

#include <imgui.h>

#include <ode/renderer-api.h>
#include <ode-essentials.h>
#include <ode-logic.h>
#include <ode-renderer.h>

using namespace ode;

namespace {
const ImU32 IM_COLOR_LIGHT_BLUE = 4294941081;
}

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

void drawDesignViewWidget(const ODE_Bitmap &bmp,
                          DesignEditorRenderer &renderer,
                          DesignEditorContext::Textures &texturesContext,
                          DesignEditorContext::Canvas &canvasContext) {
    ImGui::Begin("Interactive Design View");

    if (bmp.width > 0 && bmp.height > 0) {
        ode::Bitmap bitmap(PixelFormat::PREMULTIPLIED_RGBA, reinterpret_cast<const void*>(bmp.pixels), bmp.width, bmp.height);

        const ScaledBounds placement {0,0,static_cast<double>(bitmap.width()),static_cast<double>(bitmap.height())};

        const auto toCanvasSpace = [&canvasContext](const ImVec2 &posInScreenSpace)->ImVec2 {
            return ImVec2 {
                (posInScreenSpace.x - canvasContext.bbMin.x) / canvasContext.bbSize.x,
                (posInScreenSpace.y - canvasContext.bbMin.y) / canvasContext.bbSize.y,
            };
        };

        SelectionRectangleOpt selectionRectangle = std::nullopt;
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

        texturesContext.designImageTexture = renderer.blendImageToTexture(std::move(bitmap), placement, 2, selectionRectangle);

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
