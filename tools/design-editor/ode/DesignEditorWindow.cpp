
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
#include "widgets/DesignEditorToolbarWidget.h"
#include "widgets/DesignEditorLayerListWidget.h"
#include "widgets/DesignEditorDesignViewWidget.h"
#include "widgets/DesignEditorLayerPropertiesWidget.h"
#include "widgets/DesignEditorUIHelpers.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
    glfwFocusWindow(window);
    if (count == 1) {
        DesignEditorWindow &rgiWindow = DesignEditorWindow::getInstance();
        rgiWindow.readOctopusFile(paths[0]);
    }
}

std::vector<ODE_StringRef> listChildLayers(const ODE_LayerList &layerList,
                                           const ODE_StringRef &rootLayerId) {
    std::vector<ODE_StringRef> childLayerIds;

    for (int i = 0; i < layerList.n; i++) {
        const ODE_LayerList::Entry &entry = layerList.entries[i];
        if (strcmp(entry.parentId.data, rootLayerId.data) == 0) {
            childLayerIds.emplace_back(entry.id);
        }
    }

    return childLayerIds;
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

    /// Graphics context accessor
    GraphicsContext *gc(DesignEditorContext::Api &apiContext) {
        return reinterpret_cast<GraphicsContext *>(apiContext.rc.ptr);
    }

    int initialize() {
        CHECK(ode_initializeEngineAttributes(&context.api.engineAttribs));
        CHECK(ode_createEngine(&context.api.engine, &context.api.engineAttribs));
        CHECK(ode_createRendererContext(context.api.engine, &context.api.rc, ode_stringRef("Design Editor")));
        renderer = std::make_unique<DesignEditorRenderer>();
        return 0;
    }

    int loadMissingFonts(const FilePath &fontDir) {
        ODE_StringList missingFonts;
        CHECK(ode_design_listMissingFonts(context.api.design, &missingFonts));

        for (int i = 0; i < missingFonts.n; ++i) {
            if (missingFonts.entries[i].length > 0) {
                const std::string pathStr = (std::string)fontDir+std::string("/")+ode_stringDeref(missingFonts.entries[i])+std::string(".ttf");
                CHECK(ode_design_loadFontFile(context.api.design, missingFonts.entries[i], ode_stringRef(pathStr), ODE_StringRef()));
            }
        }

        return 0;
    }

    int loadOctopus(const FilePath &octopusPath,
                    const FilePath &fontDir) {
        context.layerSelection.clear();

        if (!readFile(octopusPath, context.loadedOctopus.octopusJson)) {
            fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
            return false;
        }

        if (context.api.design.ptr) {
            CHECK(ode_destroyDesign(context.api.design));
        }

        CHECK(ode_createDesign(context.api.engine, &context.api.design));

        // TODO: Image base directory:
        CHECK(ode_createDesignImageBase(context.api.rc, context.api.design, &context.api.imageBase));
        reinterpret_cast<ImageBase *>(context.api.imageBase.ptr)->setImageDirectory(octopusPath.parent());

        CHECK(ode_design_addComponentFromOctopusString(context.api.design, &context.api.component, context.api.metadata, ode_stringRef(context.loadedOctopus.octopusJson), nullptr));

        CHECK(loadMissingFonts(fontDir));

        CHECK(ode_component_listLayers(context.api.component, &context.api.layerList));

        GLFWwindow *window = gc(context.api)->getNativeHandle<GLFWwindow *>();
        glfwGetFramebufferSize(window, &context.api.frameView.width, &context.api.frameView.height);
        context.api.frameView.scale = 1;
        CHECK(ode_pr1_drawComponent(context.api.rc, context.api.component, context.api.imageBase, &context.api.bitmap, &context.api.frameView));

        return 0;
    }

    int loadManifest(const std::string &manifestPath) {
        if (context.api.design.ptr) {
            CHECK(ode_destroyDesign(context.api.design));
        }

        CHECK(ode_createDesign(context.api.engine, &context.api.design));

        const ODE_String manifestPathStr = ode_makeString(manifestPath);

        ODE_ParseError parseError;
        CHECK(ode_design_loadManifestFile(context.api.design, ode_stringRef(manifestPathStr), &parseError));
        return 0;
    }

    int destroy() {
        CHECK(ode_destroyDesignImageBase(context.api.imageBase));
        CHECK(ode_destroyRendererContext(context.api.rc));
        CHECK(ode_destroyDesign(context.api.design));
        CHECK(ode_destroyEngine(context.api.engine));
        return 0;
    }

    ODE_Vector2 toImageSpace(const ImVec2 &posInScreenSpace) {
        // Image space position from top layer bounds
        const ODE_StringRef &topLayerId = context.api.layerList.entries[0].id;

        const ImVec2 inCanvasSpace = ImVec2 {
            (posInScreenSpace.x - context.canvas.bbMin.x) / context.canvas.bbSize.x,
            (posInScreenSpace.y - context.canvas.bbMin.y) / context.canvas.bbSize.y,
        };

        ODE_LayerMetrics topLayerMetrics;
        ode_component_getLayerMetrics(context.api.component, topLayerId, &topLayerMetrics);

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
    data->initialize();
}

DesignEditorWindow::~DesignEditorWindow() {
    data->destroy();
}

int DesignEditorWindow::display() {
    // Get the GLFW window as initialized in ODE GraphicsContext
    GLFWwindow *window = data->gc(data->context.api)->getNativeHandle<GLFWwindow *>();

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

        // Mouse click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && data->context.canvas.isMouseOver) {
            const ODE_Vector2 mousePosImageSpace = data->toImageSpace(ImGui::GetMousePos());

            // If a single group or mask group is selected, insert into it, otherwise to the top layer
            const ODE_StringRef &topLayerID = data->context.api.layerList.entries[0].id;
            ODE_StringRef insertionLayerId = topLayerID;
            const std::vector<ODE_StringRef> &selectedLayerIds = data->context.layerSelection.layerIDs;

            if (selectedLayerIds.size() == 1) {
                ODE_String octopusString;
                if (ode_component_getOctopus(data->context.api.component, &octopusString) == ODE_RESULT_OK) {
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
            if (data->context.mode == DesignEditorMode::ADD_RECTANGLE ||
                data->context.mode == DesignEditorMode::ADD_ELLIPSE ||
                data->context.mode == DesignEditorMode::ADD_TEXT) {
                const Vector2f newLayerSize { 100, 50 };

                const octopus::Octopus octopus = [mode = data->context.mode, &layerList = data->context.api.layerList, &newLayerSize]()->octopus::Octopus {
                    if (mode == DesignEditorMode::ADD_RECTANGLE) {
                        ode::octopus_builder::ShapeLayer rectangleShape(0, 0, newLayerSize.x, newLayerSize.y);
                        rectangleShape.setColor(Color(0.5, 0.5, 0.5, 1.0));
                        const std::optional<std::string> idOpt = findAvailableLayerId("SHAPE", layerList);
                        if (idOpt.has_value()) {
                            rectangleShape.id = *idOpt;
                            rectangleShape.name = *idOpt;
                        }
                        return ode::octopus_builder::buildOctopus("Rectangle", rectangleShape);

                    } else if (mode == DesignEditorMode::ADD_ELLIPSE) {
                        const std::string w = std::to_string(newLayerSize.x);
                        const std::string ratio = std::to_string(newLayerSize.x / newLayerSize.y);
                        ode::octopus_builder::ShapeLayer ellipseShape(0, 0, newLayerSize.x, newLayerSize.y);
                        ellipseShape.setPath("M 0,0 a " + ratio + " 1 0 0 0 " + w + " 0 a " + ratio + " 1 0 0 0 -" + w + " 0");
                        const std::optional<std::string> idOpt = findAvailableLayerId("ELLIPSE", layerList);
                        if (idOpt.has_value()) {
                            ellipseShape.id = *idOpt;
                            ellipseShape.name = *idOpt;
                        }
                        return ode::octopus_builder::buildOctopus("Ellipse", ellipseShape);

                    } else {
                        ode::octopus_builder::TextLayer textShape("Text");
                        textShape.setColor(Color(0.5, 0.5, 0.5, 1.0));
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
                const ODE_Result result = ode_component_addLayer(data->context.api.component, insertionLayerId, {}, ode_stringRef(octopusLayerJson), &parseError);

                if (result == ODE_RESULT_OK) {
                    data->loadMissingFonts(fontDirectory);

                    const ODE_Transformation translation { 1, 0, 0, 1, std::round(mousePosImageSpace.x), std::round(mousePosImageSpace.y) };
                    ode_component_listLayers(data->context.api.component, &data->context.api.layerList);
                    const ODE_StringRef insertedLayerId = lastChildLayerId(data->context.api.layerList, insertionLayerId);
                    if (insertedLayerId.data!=nullptr && insertedLayerId.length>=0) {
                        ode_component_transformLayer(data->context.api.component, insertedLayerId, ODE_TRANSFORMATION_BASIS_LAYER, translation);
                    }

                    ode_pr1_drawComponent(data->context.api.rc, data->context.api.component, data->context.api.imageBase, &data->context.api.bitmap, &data->context.api.frameView);
                }

            } else if (data->context.mode == DesignEditorMode::SELECT) {
                ODE_String selectedLayerId;
                ode_component_identifyLayer(data->context.api.component, &selectedLayerId, mousePosImageSpace, 2.0f);
                if (isImGuiMultiselectKeyModifierPressed()) {
                    data->context.layerSelection.add(selectedLayerId.data);
                } else {
                    data->context.layerSelection.select(selectedLayerId.data);
                }
            }
        }

        // Mouse rectangle selection
        if (data->context.mode == DesignEditorMode::SELECT &&
            data->context.canvas.mouseClickPos.has_value() &&
            data->context.canvas.mouseDragPos.has_value()) {
            const ODE_Vector2 rectStartInImageSpace = data->toImageSpace(*data->context.canvas.mouseClickPos);
            const ODE_Vector2 rectEndInImageSpace = data->toImageSpace(*data->context.canvas.mouseDragPos);
            const ODE_Rectangle selectionRectangle {
                ODE_Vector2 { std::min(rectStartInImageSpace.x, rectEndInImageSpace.x), std::min(rectStartInImageSpace.y, rectEndInImageSpace.y) },
                ODE_Vector2 { std::max(rectStartInImageSpace.x, rectEndInImageSpace.x), std::max(rectStartInImageSpace.y, rectEndInImageSpace.y) } };

            data->context.layerSelection.clear();

            for (int i = 0; i < data->context.api.layerList.n; ++i) {
                const ODE_LayerList::Entry &layer = data->context.api.layerList.entries[i];

                ODE_LayerMetrics layerMetrics;
                ode_component_getLayerMetrics(data->context.api.component, layer.id, &layerMetrics);
                const ODE_Rectangle &layerBounds = layerMetrics.transformedGraphicalBounds;

                if (isRectangleIntersection(selectionRectangle, layerBounds)) {
                    data->context.layerSelection.add(layer.id.data);
                }
            }
        }

        if (data->context.canvas.isMouseOver) {
            const ODE_Vector2 mousePosImageSpace = data->toImageSpace(ImGui::GetMousePos());
            ImGui::SetTooltip("[%.2f, %.2f]", mousePosImageSpace.x, mousePosImageSpace.y);
        }

        // ODE DesignEditor controls window
        drawControlsWidget();

        if (data->context.widgets.showToolbar) {
            drawToolbarWidget(data->context.api, data->context.api.layerList, data->context.layerSelection, data->context.mode);
        }
        if (data->context.widgets.showLayerList) {
            drawLayerListWidget(data->context.api.layerList, data->context.layerSelection);
        }
        if (data->context.widgets.showDesignView) {
            drawDesignViewWidget(data->context.api, *data->renderer, data->context.textures, data->context.canvas, data->context.layerSelection, data->context.api.layerList.entries[0].id, data->context.imageVisualizationParams.selectedDisplayMode);
        }
        if (data->context.widgets.showLayerProperties) {
            drawLayerPropertiesWidget(data->context.api, data->context.loadedOctopus, data->context.layerSelection, data->context.fileDialog);
        }

        if (data->context.widgets.showImGuiDebugger) {
            ImGui::ShowMetricsWindow();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const ImVec4 &clearColor = data->context.imGuiWindow.clearColor;
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

bool DesignEditorWindow::readManifestFile(const FilePath &manifestPath) {
    const int loadError = data->loadManifest((std::string)manifestPath);
    return loadError == 0;
}

bool DesignEditorWindow::readOctopusFile(const FilePath &octopusPath) {
    data->context.loadedOctopus.clear();
    data->context.loadedOctopus.filePath = octopusPath;

    const int loadError = data->loadOctopus(octopusPath, fontDirectory);
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
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Choose Octopus *.json File", filters, data->context.fileDialog.octopusFilePath, data->context.fileDialog.octopusFileName);
    }

    if (data->context.loadedOctopus.isLoaded()) {
        // Open "Save Graphiz File" file dialog on button press
        if (ImGui::Button("Save Octopus File")) {
            const char* filters = ".json";
            ImGuiFileDialog::Instance()->OpenDialog("SaveOctopusFileDlgKey", "Save as *.json", filters, data->context.fileDialog.octopusFilePath, data->context.fileDialog.octopusFileName);
        }
    }

    // Display "Open Octopus File" file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseOctopusFileDlgKey")) {
        data->context.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        data->context.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

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

    if (data->context.loadedOctopus.isLoaded()) {
        // Display "Save Octopus File" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveOctopusFileDlgKey")) {
            data->context.fileDialog.octopusFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->context.fileDialog.octopusFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                ODE_String octopusString;
                ode_component_getOctopus(data->context.api.component, &octopusString);

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
    ImGui::Checkbox("Toolbar", &data->context.widgets.showToolbar);
    ImGui::Checkbox("Layer List", &data->context.widgets.showLayerList);
    ImGui::Checkbox("Interactive Design View", &data->context.widgets.showDesignView);
    ImGui::Checkbox("Layer Properties", &data->context.widgets.showLayerProperties);

    ImGui::NextColumn();

    ImGui::Text("Blend Mode");
    ImGui::Combo(" ", &data->context.imageVisualizationParams.selectedDisplayMode, designEditorImageDisplayModes, sizeof(designEditorImageDisplayModes)/sizeof(designEditorImageDisplayModes[0]));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Frame rate:  %f", ImGui::GetIO().Framerate);

    ImGui::NextColumn();

    ImGui::Checkbox("ImGui Metrics/Debugger", &data->context.widgets.showImGuiDebugger);

    ImGui::End();
}

void DesignEditorWindow::handleKeyboardEvents() {
    const float zoomKeySpeed = 0.03f;
    const float minZoom = 1.0f;
    const float maxZoom = 10.0f;

    if (ImGui::IsKeyPressed(ImGuiKey_LeftSuper)) {
        if (ImGui::IsKeyPressed(ImGuiKey_1)) {
            data->context.mode = DesignEditorMode::SELECT;
        } else if (ImGui::IsKeyPressed(ImGuiKey_2)) {
            data->context.mode = DesignEditorMode::ADD_RECTANGLE;
        } else if (ImGui::IsKeyPressed(ImGuiKey_3)) {
            data->context.mode = DesignEditorMode::ADD_ELLIPSE;
        } else if (ImGui::IsKeyPressed(ImGuiKey_4)) {
            data->context.mode = DesignEditorMode::ADD_TEXT;
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        data->context.canvas.zoom = std::clamp(data->context.canvas.zoom + zoomKeySpeed, minZoom, maxZoom);
    } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
        data->context.canvas.zoom = std::clamp(data->context.canvas.zoom - zoomKeySpeed, minZoom, maxZoom);
    }
}
