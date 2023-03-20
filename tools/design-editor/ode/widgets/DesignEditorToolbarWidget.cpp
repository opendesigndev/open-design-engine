
#include "DesignEditorToolbarWidget.h"

#include <imgui.h>

namespace {
const ImU32 IM_COLOR_DARK_RED = 4278190233;
const ImU32 IM_COLOR_LIGHT_BLUE = 4294941081;
}

void drawToolbarWidget(DesignEditorMode &mode) {
    ImGui::Begin("Toolbar");

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::SELECT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Select")) {
        mode = DesignEditorMode::SELECT;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_RECTANGLE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Rectangle")) {
        mode = DesignEditorMode::ADD_RECTANGLE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_ELLIPSE ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Ellipse")) {
        mode = DesignEditorMode::ADD_ELLIPSE;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, mode == DesignEditorMode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mode == DesignEditorMode::ADD_TEXT ? IM_COLOR_DARK_RED : IM_COLOR_LIGHT_BLUE);
    if (ImGui::Button("Add Text")) {
        mode = DesignEditorMode::ADD_TEXT;
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    ImGui::End();
}
