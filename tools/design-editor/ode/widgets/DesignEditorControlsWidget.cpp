
#include "DesignEditorControlsWidget.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "../DesignEditorWindow.h"

void drawControlsWidget(DesignEditorDesign &design,
                        DesignEditorUIState &ui) {
    ImGui::SetNextWindowSize(ImVec2(420, 170), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Controls");

    ImGui::Columns(3);

    // Create "New Design"
    if (ImGui::Button("New")) {
        DesignEditorWindow::getInstance().createEmptyDesign();
    }

    // Open "Open Octopus File" file dialog on button press
    if (ImGui::Button("Open")) {
        const char* filters = "Octopus files (*.octopus){.octopus},Octopus component files (*.json){.json}";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Open a file", filters, ui.fileDialog.octopusFilePath, ui.fileDialog.octopusFileName);
    }

    if (!design.empty()) {
        // Open "Save Octopus File" file dialog on button press
        if (ImGui::Button("Save")) {
            const char* filters = ".octopus";
            ImGuiFileDialog::Instance()->OpenDialog("SaveOctopusFileDlgKey", "Save as *.octopus file", filters, ui.fileDialog.octopusFilePath, ui.fileDialog.octopusFileName);
        }
    }

    // Display "Open Octopus File" file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseOctopusFileDlgKey")) {
        ui.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        ui.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

        if (ImGuiFileDialog::Instance()->IsOk()) {
            const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

            // Handle file input
            const bool isOctopusFileRead = DesignEditorWindow::getInstance().readOctopusFile(filePathName);
            if (!isOctopusFileRead) {
                fprintf(stderr, "Failed to parse Octopus file\n");
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (!design.empty()) {
        // Display "Save Octopus File" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveOctopusFileDlgKey")) {
            ui.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            ui.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                const ODE_ComponentHandle &component = design.components.back().component;

                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                ODE_String octopusString;
                ode_component_getOctopus(component, &octopusString);

                const bool isSaved = ode::writeFile(filePathName, octopusString.data, octopusString.length);
                if (!isSaved) {
                    fprintf(stderr, "Internal error (saving Octopus json to filesystem)\n");
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    ImGui::NextColumn();

    ImGui::Text("Widgets:");
    ImGui::Checkbox("Toolbar", &ui.widgets.showToolbar);
    ImGui::Checkbox("Layer List", &ui.widgets.showLayerList);
    ImGui::Checkbox("Interactive Design View", &ui.widgets.showDesignView);
    ImGui::Checkbox("Layer Properties", &ui.widgets.showLayerProperties);

    ImGui::NextColumn();

    ImGui::Text("Blend Mode");
    ImGui::Combo(" ", &ui.imageVisualizationParams.selectedDisplayMode, designEditorImageDisplayModes, sizeof(designEditorImageDisplayModes)/sizeof(designEditorImageDisplayModes[0]));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Frame rate:  %f", ImGui::GetIO().Framerate);

    ImGui::NextColumn();

    ImGui::Checkbox("ImGui Metrics/Debugger", &ui.widgets.showImGuiDebugger);

    ImGui::End();
}
