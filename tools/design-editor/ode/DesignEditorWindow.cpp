
#include "DesignEditorWindow.h"

// ImGui includes
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// ImGuiFileDialog includes
#include <ImGuiFileDialog.h>

// OD includes
#include <ode-renderer.h>
#include <ode-diagnostics.h>
#include <ode-media.h>
#include <ode/renderer-api.h>
#include "DesignEditorRenderer.h"
#include "DesignEditorContext.h"
#include "DesignEditorUIState.h"
#include "widgets/DesignEditorToolbarWidget.h"
#include "widgets/DesignEditorLayerListWidget.h"
#include "widgets/DesignEditorDesignViewWidget.h"
#include "widgets/DesignEditorLayerPropertiesWidget.h"
#include "widgets/DesignEditorUIHelpers.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)

const Vector2f DEFAULT_NEW_SHAPE_SIZE { 100, 50 };
const Color DEFAULT_NEW_SHAPE_COLOR(0.5, 0.5, 0.5, 1.0);

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
    glfwFocusWindow(window);
    if (count == 1) {
        DesignEditorWindow &rgiWindow = DesignEditorWindow::getInstance();
        rgiWindow.readOctopusFile(paths[0]);
    }
}

ODE_StringRef lastChildLayerId(const ODE_LayerList &layerList,
                               const ODE_StringRef &rootLayerId) {
    if (layerList.n <= 0) {
        return ODE_StringRef { nullptr, 0 };
    }

    for (int i = layerList.n - 1; i >=0; --i) {
        const ODE_LayerList::Entry &entry = layerList.entries[i];
        if (strcmp(entry.parentId.data, rootLayerId.data) == 0) {
            return entry.id;
        }
    }

    return ODE_StringRef { nullptr, 0 };
}

}

struct DesignEditorWindow::Internal {
    /// Renderer
    std::unique_ptr<DesignEditorRenderer> renderer;

    /// Context
    DesignEditorContext context;
    /// UI
    DesignEditorUIState ui;

    /// Graphics context accessor
    GraphicsContext *gc() {
        return reinterpret_cast<GraphicsContext *>(context.rc.ptr);
    }

    int loadMissingFonts(const FilePath &fontDir) {
        ODE_StringList missingFonts;
        CHECK(ode_design_listMissingFonts(context.design.design, &missingFonts));

        for (int i = 0; i < missingFonts.n; ++i) {
            if (missingFonts.entries[i].length > 0) {
                const std::string pathStr = (std::string)fontDir+std::string("/")+ode_stringDeref(missingFonts.entries[i])+std::string(".ttf");
                CHECK(ode_design_loadFontFile(context.design.design, missingFonts.entries[i], ode_stringRef(pathStr), ODE_StringRef()));
            }
        }
        return 0;
    }

    int reloadOctopus(const FilePath &octopusPath,
                      const FilePath &fontDir) {
        ui.layerSelection.clear();

        // TODO: Allow multiple components
        if (context.design.design.ptr) {
            CHECK(ode_destroyDesign(context.design.design));
            CHECK(ode_createDesign(context.engine, &context.design.design));
        }

        context.design.components.clear();
        DesignEditorComponent &newComponent = context.design.components.emplace_back();
        newComponent.filePath = octopusPath;

        if (!readFile(newComponent.filePath, newComponent.octopusJson)) {
            fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
            return false;
        }

        // TODO: Image base directory:
        reinterpret_cast<ImageBase *>(context.design.imageBase.ptr)->setImageDirectory(octopusPath.parent());

        CHECK(ode_design_addComponentFromOctopusString(context.design.design, &newComponent.component, newComponent.metadata, ode_stringRef(newComponent.octopusJson), nullptr));
        CHECK(loadMissingFonts(fontDir));
        CHECK(ode_component_listLayers(newComponent.component, &newComponent.layerList));
        CHECK(ode_pr1_drawComponent(context.rc, newComponent.component, context.design.imageBase, &newComponent.bitmap, &context.frameView));
        return 0;
    }

    ODE_Vector2 toImageSpace(const ImVec2 &posInScreenSpace, const DesignEditorComponent &component) {
        // Image space position from top layer bounds
        const ODE_StringRef &topLayerId = component.layerList.entries[0].id;

        const ImVec2 inCanvasSpace = ImVec2 {
            (posInScreenSpace.x - ui.canvas.bbMin.x) / ui.canvas.bbSize.x,
            (posInScreenSpace.y - ui.canvas.bbMin.y) / ui.canvas.bbSize.y,
        };

        ODE_LayerMetrics topLayerMetrics;
        ode_component_getLayerMetrics(component.component, topLayerId, &topLayerMetrics);

        const ODE_Rectangle &topLayerBounds = topLayerMetrics.logicalBounds;
        const ODE_Vector2 imageSpacePosition {
            inCanvasSpace.x * topLayerBounds.b.x,
            inCanvasSpace.y * topLayerBounds.b.y
        };

        return imageSpacePosition;
    }

};


/*static*/ DesignEditorWindow& DesignEditorWindow::getInstance() {
    static DesignEditorWindow instance;
    return instance;
}

DesignEditorWindow::DesignEditorWindow() :
    data(new Internal()) {
    data->context.initialize();
    data->renderer = std::make_unique<DesignEditorRenderer>();
}

DesignEditorWindow::~DesignEditorWindow() {
    data->context.destroy();
}

int DesignEditorWindow::display() {
    // Get the GLFW window as initialized in ODE GraphicsContext
    GLFWwindow *window = data->gc()->getNativeHandle<GLFWwindow *>();

    // Set file drop callback
    glfwSetDropCallback(window, fileDropCallback);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Keyboard events (move slider, zoom)
        handleKeyboardEvents();

        // ODE DesignEditor controls window
        drawControlsWidget();

        if (!data->context.design.empty()) {
            DesignEditorComponent &component = data->context.design.components.back();

            // Mouse click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && data->ui.canvas.isMouseOver) {
                const ODE_Vector2 mousePosImageSpace = data->toImageSpace(ImGui::GetMousePos(), component);

                // If a single group or mask group is selected, insert into it, otherwise to the top layer
                const ODE_StringRef &topLayerID = component.layerList.entries[0].id;
                ODE_StringRef insertionLayerId = topLayerID;
                const std::vector<ODE_StringRef> &selectedLayerIds = data->ui.layerSelection.layerIDs;

                if (selectedLayerIds.size() == 1) {
                    ODE_String octopusString;
                    if (ode_component_getOctopus(component.component, &octopusString) == ODE_RESULT_OK) {
                        octopus::Octopus componentOctopus;
                        octopus::Parser::parse(componentOctopus, octopusString.data);

                        if (componentOctopus.content.has_value()) {
                            const ODE_StringRef &selectedLayerId = selectedLayerIds.front();
                            const octopus::Layer *layer = findLayer(*componentOctopus.content, ode_stringDeref(selectedLayerId));

                            if (layer != nullptr && (layer->type == octopus::Layer::Type::GROUP || layer->type == octopus::Layer::Type::MASK_GROUP)) {
                                insertionLayerId = selectedLayerId;
                            }
                        }
                    }
                }

                // Layer insertion
                if (data->ui.mode == DesignEditorUIState::Mode::ADD_RECTANGLE ||
                    data->ui.mode == DesignEditorUIState::Mode::ADD_ELLIPSE ||
                    data->ui.mode == DesignEditorUIState::Mode::ADD_TEXT) {
                    const octopus::Octopus octopus = [mode = data->ui.mode, &layerList = component.layerList]()->octopus::Octopus {
                        if (mode == DesignEditorUIState::Mode::ADD_RECTANGLE) {
                            ode::octopus_builder::ShapeLayer rectangleShape(0, 0, DEFAULT_NEW_SHAPE_SIZE.x, DEFAULT_NEW_SHAPE_SIZE.y);
                            rectangleShape.setColor(DEFAULT_NEW_SHAPE_COLOR);
                            const std::optional<std::string> idOpt = findAvailableLayerId("SHAPE", layerList);
                            if (idOpt.has_value()) {
                                rectangleShape.id = *idOpt;
                                rectangleShape.name = *idOpt;
                            }
                            return ode::octopus_builder::buildOctopus("Rectangle", rectangleShape);

                        } else if (mode == DesignEditorUIState::Mode::ADD_ELLIPSE) {
                            const std::string w = std::to_string(DEFAULT_NEW_SHAPE_SIZE.x);
                            const std::string ratio = std::to_string(DEFAULT_NEW_SHAPE_SIZE.x / DEFAULT_NEW_SHAPE_SIZE.y);
                            ode::octopus_builder::ShapeLayer ellipseShape(0, 0, DEFAULT_NEW_SHAPE_SIZE.x, DEFAULT_NEW_SHAPE_SIZE.y);
                            ellipseShape.setPath("M 0,0 a " + ratio + " 1 0 0 0 " + w + " 0 a " + ratio + " 1 0 0 0 -" + w + " 0");
                            ellipseShape.setColor(DEFAULT_NEW_SHAPE_COLOR);
                            const std::optional<std::string> idOpt = findAvailableLayerId("ELLIPSE", layerList);
                            if (idOpt.has_value()) {
                                ellipseShape.id = *idOpt;
                                ellipseShape.name = *idOpt;
                            }
                            return ode::octopus_builder::buildOctopus("Ellipse", ellipseShape);

                        } else {
                            ode::octopus_builder::TextLayer textShape("Text");
                            textShape.setColor(DEFAULT_NEW_SHAPE_COLOR);
                            const std::optional<std::string> idOpt = findAvailableLayerId("TEXT", layerList);
                            if (idOpt.has_value()) {
                                textShape.id = *idOpt;
                                textShape.name = *idOpt;
                            }
                            return ode::octopus_builder::buildOctopus("Text", textShape);
                        }
                    } ();

                    std::string octopusLayerJson;
                    octopus::Serializer::serialize(octopusLayerJson, *octopus.content);

                    ODE_ParseError parseError;
                    const ODE_Result result = ode_component_addLayer(component.component, insertionLayerId, {}, ode_stringRef(octopusLayerJson), &parseError);

                    if (result == ODE_RESULT_OK) {
                        data->loadMissingFonts(fontDirectory);

                        const ODE_Transformation translation { 1, 0, 0, 1, std::round(mousePosImageSpace.x), std::round(mousePosImageSpace.y) };
                        ode_component_listLayers(component.component, &component.layerList);
                        const ODE_StringRef insertedLayerId = lastChildLayerId(component.layerList, insertionLayerId);
                        if (insertedLayerId.data!=nullptr && insertedLayerId.length>=0) {
                            ode_component_transformLayer(component.component, insertedLayerId, ODE_TRANSFORMATION_BASIS_LAYER, translation);
                        }

                        ode_pr1_drawComponent(data->context.rc, component.component, data->context.design.imageBase, &component.bitmap, &data->context.frameView);
                    }

                } else if (data->ui.mode == DesignEditorUIState::Mode::SELECT) {
                    ODE_String selectedLayerId;
                    ode_component_identifyLayer(component.component, &selectedLayerId, mousePosImageSpace, 2.0f);
                    if (isImGuiMultiselectKeyModifierPressed()) {
                        data->ui.layerSelection.add(selectedLayerId.data);
                    } else {
                        data->ui.layerSelection.select(selectedLayerId.data);
                    }
                }
            }

            // Mouse rectangle selection
            if (data->ui.mode == DesignEditorUIState::Mode::SELECT &&
                data->ui.canvas.mouseClickPos.has_value() &&
                data->ui.canvas.mouseDragPos.has_value()) {
                const ODE_Vector2 rectStartInImageSpace = data->toImageSpace(*data->ui.canvas.mouseClickPos, component);
                const ODE_Vector2 rectEndInImageSpace = data->toImageSpace(*data->ui.canvas.mouseDragPos, component);
                const ODE_Rectangle selectionRectangle {
                    ODE_Vector2 { std::min(rectStartInImageSpace.x, rectEndInImageSpace.x), std::min(rectStartInImageSpace.y, rectEndInImageSpace.y) },
                    ODE_Vector2 { std::max(rectStartInImageSpace.x, rectEndInImageSpace.x), std::max(rectStartInImageSpace.y, rectEndInImageSpace.y) } };

                data->ui.layerSelection.clear();

                for (int i = 0; i < component.layerList.n; ++i) {
                    const ODE_LayerList::Entry &layer = component.layerList.entries[i];

                    ODE_LayerMetrics layerMetrics;
                    ode_component_getLayerMetrics(component.component, layer.id, &layerMetrics);
                    const ODE_Rectangle &layerBounds = layerMetrics.transformedGraphicalBounds;

                    if (isRectangleIntersection(selectionRectangle, layerBounds)) {
                        data->ui.layerSelection.add(layer.id.data);
                    }
                }
            }

            if (data->ui.canvas.isMouseOver) {
                const ODE_Vector2 mousePosImageSpace = data->toImageSpace(ImGui::GetMousePos(), component);
                ImGui::SetTooltip("[%.2f, %.2f]", mousePosImageSpace.x, mousePosImageSpace.y);
            }

            if (data->ui.widgets.showToolbar) {
                drawToolbarWidget(data->context, component, data->ui.layerSelection, data->ui.mode);
            }
            if (data->ui.widgets.showLayerList) {
                drawLayerListWidget(component.layerList, data->ui.layerSelection);
            }
            if (data->ui.widgets.showDesignView) {
                drawDesignViewWidget(component.component, component.bitmap, *data->renderer, data->ui.textures, data->ui.canvas, data->ui.layerSelection, component.layerList.entries[0].id, data->ui.imageVisualizationParams.selectedDisplayMode);
            }
            if (data->ui.widgets.showLayerProperties) {
                drawLayerPropertiesWidget(data->context, component, data->ui.layerSelection, data->ui.fileDialog);
            }
        }

        if (data->ui.widgets.showImGuiDebugger) {
            ImGui::ShowMetricsWindow();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const ImVec4 &clearColor = data->ui.imGuiWindow.clearColor;
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // ImGui:
        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glUseProgram(0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glUseProgram(last_program);

        glfwSwapBuffers(window);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

bool DesignEditorWindow::readOctopusFile(const FilePath &octopusPath) {
    const int loadError = data->reloadOctopus(octopusPath, fontDirectory);
    return loadError == 0;
}

void DesignEditorWindow::setImageDirectory(const FilePath &imageDirectory_) {
    imageDirectory = imageDirectory_;
}

void DesignEditorWindow::setFontDirectory(const FilePath &fontDirectory_) {
    fontDirectory = fontDirectory_;
}

void DesignEditorWindow::setIgnoreValidation(bool ignoreValidation_) {
    ignoreValidation = ignoreValidation_;
}

void DesignEditorWindow::drawControlsWidget() {
    ImGui::Begin("Controls");

    ImGui::Columns(3);

    // Open "Open Octopus File" file dialog on button press
    if (ImGui::Button("Open Octopus File")) {
        const char* filters = ".json";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Choose Octopus *.json File", filters, data->ui.fileDialog.octopusFilePath, data->ui.fileDialog.octopusFileName);
    }

    if (!data->context.design.empty()) {
        // Open "Save Octopus File" file dialog on button press
        if (ImGui::Button("Save Octopus File")) {
            const char* filters = ".json";
            ImGuiFileDialog::Instance()->OpenDialog("SaveOctopusFileDlgKey", "Save as *.json", filters, data->ui.fileDialog.octopusFilePath, data->ui.fileDialog.octopusFileName);
        }
    }

    // Display "Open Octopus File" file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseOctopusFileDlgKey")) {
        data->ui.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        data->ui.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

        if (ImGuiFileDialog::Instance()->IsOk()) {
            const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

            // Handle file input
            const bool isOctopusFileRead = readOctopusFile(filePathName);
            if (!isOctopusFileRead) {
                fprintf(stderr, "Failed to parse Octopus file\n");
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (!data->context.design.empty()) {
        // Display "Save Octopus File" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveOctopusFileDlgKey")) {
            data->ui.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->ui.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                const ODE_ComponentHandle &component = data->context.design.components.back().component;

                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                ODE_String octopusString;
                ode_component_getOctopus(component, &octopusString);

                const bool isSaved = writeFile(filePathName, octopusString.data, octopusString.length);
                if (!isSaved) {
                    fprintf(stderr, "Internal error (saving Octopus json to filesystem)\n");
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    ImGui::NextColumn();

    ImGui::Text("Widgets:");
    ImGui::Checkbox("Toolbar", &data->ui.widgets.showToolbar);
    ImGui::Checkbox("Layer List", &data->ui.widgets.showLayerList);
    ImGui::Checkbox("Interactive Design View", &data->ui.widgets.showDesignView);
    ImGui::Checkbox("Layer Properties", &data->ui.widgets.showLayerProperties);

    ImGui::NextColumn();

    ImGui::Text("Blend Mode");
    ImGui::Combo(" ", &data->ui.imageVisualizationParams.selectedDisplayMode, designEditorImageDisplayModes, sizeof(designEditorImageDisplayModes)/sizeof(designEditorImageDisplayModes[0]));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Frame rate:  %f", ImGui::GetIO().Framerate);

    ImGui::NextColumn();

    ImGui::Checkbox("ImGui Metrics/Debugger", &data->ui.widgets.showImGuiDebugger);

    ImGui::End();
}

void DesignEditorWindow::handleKeyboardEvents() {
    const float zoomKeySpeed = 0.03f;
    const float minZoom = 1.0f;
    const float maxZoom = 10.0f;

    if (ImGui::IsKeyPressed(ImGuiKey_LeftSuper)) {
        if (ImGui::IsKeyPressed(ImGuiKey_1)) {
            data->ui.mode = DesignEditorUIState::Mode::SELECT;
        } else if (ImGui::IsKeyPressed(ImGuiKey_2)) {
            data->ui.mode = DesignEditorUIState::Mode::ADD_RECTANGLE;
        } else if (ImGui::IsKeyPressed(ImGuiKey_3)) {
            data->ui.mode = DesignEditorUIState::Mode::ADD_ELLIPSE;
        } else if (ImGui::IsKeyPressed(ImGuiKey_4)) {
            data->ui.mode = DesignEditorUIState::Mode::ADD_TEXT;
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        data->ui.canvas.zoom = std::clamp(data->ui.canvas.zoom + zoomKeySpeed, minZoom, maxZoom);
    } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
        data->ui.canvas.zoom = std::clamp(data->ui.canvas.zoom - zoomKeySpeed, minZoom, maxZoom);
    }
}
