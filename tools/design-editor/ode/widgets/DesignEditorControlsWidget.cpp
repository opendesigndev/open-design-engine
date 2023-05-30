
#include "DesignEditorControlsWidget.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include "../DesignEditorWindow.h"

namespace {

template <typename T>
void insertUnique(std::vector<T> &containingVector, const T &insertVal) {
    if (std::find_if(containingVector.begin(), containingVector.end(), [&insertVal](const auto &v)->bool { return insertVal.refId == v.refId; }) == containingVector.end()) {
        containingVector.emplace_back(insertVal);
    }
}

template <typename T>
void insertUnique(std::vector<T> &containingVector, const std::vector<T> &insertVector) {
    for (const auto &insertVal : insertVector) {
        insertUnique(containingVector, insertVal);
    }
}

octopus::Assets listAllAssets(const octopus::Layer &layer) {
    octopus::Assets assets = {};
    if (layer.type == octopus::Layer::Type::SHAPE && layer.shape.has_value()) {
        for (const octopus::Fill &fill : layer.shape->fills) {
            if (fill.type == octopus::Fill::Type::IMAGE && fill.image.has_value()) {
                const octopus::Image &image = *fill.image;
                const std::string imageRef = ((FilePath)image.ref.value).filename();
                insertUnique(assets.images, octopus::AssetImage {
                    octopus::ResourceLocation {
                        image.ref.type == octopus::ImageRef::Type::PATH ? octopus::ResourceLocation::Type::RELATIVE : octopus::ResourceLocation::Type::EXTERNAL,
                        image.ref.value,
                        nonstd::nullopt,
                        nonstd::nullopt
                    },
                    imageRef, // asset ref Id
                    imageRef  // asset name
                });
            }
        }
    }
    if (layer.type == octopus::Layer::Type::TEXT && layer.text.has_value()) {
        if (layer.text->defaultStyle.font.has_value()) {
            const octopus::Font &font = *layer.text->defaultStyle.font;
            // TODO: AssetFont path
            insertUnique(assets.fonts, octopus::AssetFont {
                nonstd::nullopt,
                nonstd::nullopt,
                font.postScriptName,
                font.postScriptName
            });
        }
        if (layer.text->styles.has_value()) {
            for (const octopus::StyleRange &styleRange : *layer.text->styles) {
                if (styleRange.style.font.has_value()) {
                    const octopus::Font &font = *styleRange.style.font;
                    // TODO: AssetFont path
                    insertUnique(assets.fonts, octopus::AssetFont {
                        nonstd::nullopt,
                        nonstd::nullopt,
                        font.postScriptName,
                        font.postScriptName
                    });
                }
            }
        }
    }
    if (layer.layers.has_value()) {
        for (const octopus::Layer &subLayer : *layer.layers) {
            const octopus::Assets sublayerAssets = listAllAssets(subLayer);
            insertUnique(assets.images, sublayerAssets.images);
            insertUnique(assets.fonts, sublayerAssets.fonts);
        }
    }
    return assets;
}

}

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
        ImGui::SetNextWindowSize(ImVec2(1000, 500), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

        const char* filters = "Octopus files (*.octopus){.octopus},Octopus component files (*.json){.json}";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Open a file", filters, ui.fileDialog.octopusFilePath, ui.fileDialog.octopusFileName);
    }

    if (!design.empty()) {
        // Open "Save Octopus File" file dialog on button press
        if (ImGui::Button("Save")) {
            ImGui::SetNextWindowSize(ImVec2(1000, 500), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

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
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                const std::string fileName = (FilePath(filePathName)).filename();

                const size_t extPos = fileName.rfind('.', fileName.length());
                const std::string cleanFileName = (extPos == std::string::npos) ? fileName : fileName.substr(0, extPos);

                OctopusFile octopusFile;
                octopusFile.add("Octopus", " is universal design format. opendesign.dev.", MemoryFileSystem::CompressionMethod::NONE);

                octopus::OctopusManifest manifest;
                manifest.version = OCTOPUS_MANIFEST_VERSION;
                manifest.origin.name = "OD Internal Design Editor";
                manifest.origin.version = "1.0.0.";
                manifest.name = cleanFileName;

                for (const DesignEditorComponent &component : design.components) {
                    const ODE_StringRef &componentId = component.id;

                    ODE_String octopusString;
                    if (ode_component_getOctopus(component.component, &octopusString) == ODE_RESULT_OK) {
                        const std::string componentFileName = std::string("octopus-") + ode_stringDeref(componentId) + ".json";
                        if (!octopusFile.add(componentFileName, std::string(octopusString.data, octopusString.length), MemoryFileSystem::CompressionMethod::DEFLATE).has_value()) {
                            continue;
                        }

                        octopus::Octopus octopus;
                        octopus::Parser::parse(octopus, octopusString.data);

                        octopus::Component &octopusComponent = manifest.components.emplace_back();
                        octopusComponent.id = octopus.id;
                        octopusComponent.name = octopus.content->name;
                        octopusComponent.status = octopus::Status {
                            octopus::Status::Value::READY,
                            nonstd::nullopt,
                            0.0 };
                        octopusComponent.bounds = octopus::Bounds {
                            component.metadata.position.x,
                            component.metadata.position.y,
                            octopus.dimensions->width,
                            octopus.dimensions->height };
                        octopusComponent.location = octopus::ResourceLocation {
                            octopus::ResourceLocation::Type::RELATIVE,
                            componentFileName,
                            nonstd::nullopt,
                            nonstd::nullopt };
                        octopusComponent.assets = octopus.content.has_value() ? listAllAssets(*octopus.content) : octopus::Assets{};
                    }
                }

                std::string manifestJson;
                if (octopus::ManifestSerializer::serialize(manifestJson, manifest) == octopus::ManifestSerializer::Error::OK) {
                    if (octopusFile.add("octopus-manifest.json", manifestJson, MemoryFileSystem::CompressionMethod::DEFLATE).has_value()) {
                        const bool isSaved = octopusFile.save(filePathName);
                        if (!isSaved) {
                            fprintf(stderr, "Internal error (saving Octopus json to filesystem)\n");
                        }
                    }
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
