
#include "DesignEditorDesignViewWidget.h"

#include <imgui.h>

#include <ode/renderer-api.h>
#include <ode-essentials.h>
#include <ode-logic.h>
#include <ode-renderer.h>

using namespace ode;

namespace {

void drawImGuiWidgetTexture(const GLuint textureHandle, int width, int height, float &zoom, size_t colsCount = 1, size_t rowsCount = 1) {
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

    ImGui::Image((void*)(intptr_t)textureHandle, newImageSize);
}

}

void drawDesignViewWidget(DesignEditorContext &context, DesignEditorRenderer &renderer) {
    ImGui::Begin("Interactive Design View");

    const ODE_Bitmap &bmp = context.bitmap;
    if (bmp.width > 0 && bmp.height > 0) {
        ode::Bitmap bitmap(PixelFormat::PREMULTIPLIED_RGBA, reinterpret_cast<const void*>(bmp.pixels), bmp.width, bmp.height);

        const ScaledBounds placement {0,0,static_cast<double>(bitmap.width()),static_cast<double>(bitmap.height())};
        context.textures.designImageTexture = renderer.blendImageToTexture(std::move(bitmap), placement, 2);

        drawImGuiWidgetTexture(context.textures.designImageTexture->getInternalGLHandle(),
                               context.textures.designImageTexture->dimensions().x,
                               context.textures.designImageTexture->dimensions().y,
                               context.canvas.zoom);

        context.canvas.isMouseOver = ImGui::IsItemHovered();
        context.canvas.bbSize = ImGui::GetItemRectSize();
        context.canvas.bbMin = ImGui::GetItemRectMin();
        context.canvas.bbMax = ImGui::GetItemRectMax();
    }

    ImGui::End();
}
