
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
#include "DesignEditorLoadedOctopus.h"
#include "DesignEditorImageVisualizationParams.h"
#include "DesignEditorRenderer.h"
#include "DesignEditorContext.h"
#include "widgets/DesignEditorToolbarWidget.h"
#include "widgets/DesignEditorLayerListWidget.h"
#include "widgets/DesignEditorDesignViewWidget.h"
#include "widgets/DesignEditorLayerPropertiesWidget.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)

// TODO move to common API utils
static ODE_StringRef stringRef(const std::string &str) {
    ODE_StringRef ref;
    ref.data = str.c_str();
    ref.length = int(str.size());
    return ref;
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
    glfwFocusWindow(window);
    if (count == 1) {
        DesignEditorWindow &rgiWindow = DesignEditorWindow::getInstance();
        rgiWindow.readOctopusFile(paths[0]);
    }
}

}

struct DesignEditorWindow::Internal {
    Internal() = default;
    ~Internal() {
        loadedOctopus.clear();
    }

    /// Loaded octopus file data
    // TODO: Loaded Octopus
    DesignEditorLoadedOctopus loadedOctopus;

    /// Renderer
    std::unique_ptr<DesignEditorRenderer> renderer;

    /// Current and previous image visualization params
    DesignEditorImageVisualizationParams imageVisualizationParams;
    DesignEditorImageVisualizationParams prevImageVisualizationParams;

    DesignEditorContext context;

    /// Graphics context accessor
    GraphicsContext *gc(DesignEditorContext::Api &apiContext) {
        return reinterpret_cast<GraphicsContext *>(apiContext.rc.ptr);
    }

    // TODO: Move
    int initialize(DesignEditorContext::Api &apiContext) {
        CHECK(ode_initializeEngineAttributes(&apiContext.engineAttribs));
        CHECK(ode_createEngine(&apiContext.engine, &apiContext.engineAttribs));
        CHECK(ode_createRendererContext(apiContext.engine, &apiContext.rc, stringRef("Design Editor")));
        renderer = std::make_unique<DesignEditorRenderer>();
        return 0;
    }

    // TODO: Move
    int loadOctopus(const std::string &octopusJson, const FilePath &octopusPath, const FilePath &fontDir, DesignEditorContext::Api &apiContext) {
        if (apiContext.design.ptr) {
            CHECK(ode_destroyDesign(apiContext.design));
        }

        CHECK(ode_createDesign(apiContext.engine, &apiContext.design));

        // TODO: Image base:
        CHECK(ode_createDesignImageBase(apiContext.rc, apiContext.design, &apiContext.imageBase));
        reinterpret_cast<ImageBase *>(apiContext.imageBase.ptr)->setImageDirectory(octopusPath.parent());

        CHECK(ode_design_addComponentFromOctopusString(apiContext.design, &apiContext.component, apiContext.metadata, stringRef(octopusJson), nullptr));

        ODE_StringList missingFonts;
        CHECK(ode_design_listMissingFonts(apiContext.design, &missingFonts));

        for (int i = 0; i < missingFonts.n; ++i) {
            if (missingFonts.entries[i].length <= 0) {
                continue;
            }
            const std::string pathStr = (std::string)fontDir+std::string("/")+std::string(missingFonts.entries[i].data)+std::string(".ttf");
            const ODE_StringRef path { pathStr.c_str(), static_cast<int>(strlen(path.data)) };
            ode_design_loadFontFile(apiContext.design, missingFonts.entries[i], path, ODE_StringRef());
        }

        CHECK(ode_component_listLayers(apiContext.component, &loadedOctopus.layerList));

        GLFWwindow *window = gc(apiContext)->getNativeHandle<GLFWwindow *>();
        glfwGetFramebufferSize(window, &apiContext.frameView.width, &apiContext.frameView.height);
        apiContext.frameView.scale = 1;
        CHECK(ode_pr1_drawComponent(apiContext.rc, apiContext.component, apiContext.imageBase, &apiContext.bitmap, &apiContext.frameView));

        return 0;
    }

    // TODO: Move
    int loadManifest(const std::string &manifestPath, DesignEditorContext::Api &apiContext) {
        if (apiContext.design.ptr) {
            CHECK(ode_destroyDesign(apiContext.design));
        }

        CHECK(ode_createDesign(apiContext.engine, &apiContext.design));

        const ODE_String manifestPathStr = ode_makeString(manifestPath);
        ODE_ParseError parseError;

        CHECK(ode_design_loadManifestFile(apiContext.design, ode_stringRef(manifestPathStr), &parseError));
        return 0;
    }

    // TODO: Move
    int destroy(DesignEditorContext::Api &apiContext) {
        CHECK(ode_destroyDesignImageBase(apiContext.imageBase));
        CHECK(ode_destroyRendererContext(apiContext.rc));
        CHECK(ode_destroyDesign(apiContext.design));
        CHECK(ode_destroyEngine(apiContext.engine));
        return 0;
    }
};


/*static*/ DesignEditorWindow& DesignEditorWindow::getInstance() {
    static DesignEditorWindow instance;
    return instance;
}

DesignEditorWindow::DesignEditorWindow() :
    data(new Internal()) {
    data->initialize(data->context.api);
}

DesignEditorWindow::~DesignEditorWindow() {
    data->destroy(data->context.api);
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

        const bool widgetsContextChanged = data->context.widgets != data->context.preWidgets;
        const bool imageVisualizationParamsChanged = data->imageVisualizationParams != data->prevImageVisualizationParams;

        // Display result texture
        if (data->loadedOctopus.isLoaded()) {
            if (data->loadedOctopus.reloaded || widgetsContextChanged || imageVisualizationParamsChanged) {
                data->loadedOctopus.reloaded = false;

                // TODO: Display result texture
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && data->context.canvas.isMouseOver) {
            const ImVec2 mousePosInScreenSpace = ImGui::GetMousePos();
            const ImVec2 mousePosInCanvasSpace = ImVec2 {
                (mousePosInScreenSpace.x - data->context.canvas.bbMin.x) / data->context.canvas.bbSize.x,
                (mousePosInScreenSpace.y - data->context.canvas.bbMin.y) / data->context.canvas.bbSize.y,
            };

            // TODO: Is image space position from top layer bounds correct ?
            const ODE_StringRef &topLayerID = data->loadedOctopus.layerList.entries[0].id;
            ODE_LayerMetrics topLayerMetrics;
            ode_component_getLayerMetrics(data->context.api.component, topLayerID, &topLayerMetrics);
            const ODE_Rectangle &topLayerBounds = topLayerMetrics.logicalBounds;
            const ODE_Vector2 imageSpacePosition {
                mousePosInCanvasSpace.x * topLayerBounds.b.x,
                mousePosInCanvasSpace.y * topLayerBounds.b.y
            };

            if (data->mode == Internal::Mode::SELECT) {
                ODE_String selectedLayerId;
                CHECK(ode_component_identifyLayer(data->context.component, &selectedLayerId, imageSpacePosition, 2.0f));
                data->selectionContext.select(selectedLayerId);
            }
        }

        // ODE DesignEditor controls window
        drawControlsWidget();

        if (data->context.widgets.showToolbar) {
            drawToolbarWidget(data->context.mode);
        }
        if (data->context.widgets.showLayerList) {
            drawLayerListWidget(data->loadedOctopus, data->context.layerSelection);
        }
        if (data->context.widgets.showDesignView) {
            drawDesignViewWidget(data->context.api.bitmap, *data->renderer, data->context.textures, data->context.canvas);
        }
        if (data->context.widgets.showLayerProperties) {
            drawLayerPropertiesWidget(data->loadedOctopus.layerList, data->context.api, data->context.layerSelection, data->context.layerProperties);
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
    const int loadError = data->loadManifest((std::string)manifestPath, data->context.api);
    return loadError == 0;
}

bool DesignEditorWindow::readOctopusFile(const FilePath &octopusPath) {
    data->loadedOctopus.clear();
    data->loadedOctopus.filePath = octopusPath;

    if (!readFile(octopusPath, data->loadedOctopus.octopusJson)) {
        fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
        return false;
    }

    const int loadError = data->loadOctopus(data->loadedOctopus.octopusJson, octopusPath, fontDirectory, data->context.api);
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
    data->context.preWidgets = data->context.widgets;
    data->prevImageVisualizationParams = data->imageVisualizationParams;

    ImGui::Begin("Controls");

    ImGui::Columns(3);

    // Open "Open Octopus File" file dialog on button press
    if (ImGui::Button("Open Octopus File")) {
        const char* filters = ".json";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Choose Octopus *.json File", filters, data->context.fileDialog.filePath, data->context.fileDialog.fileName);
    }

    if (data->loadedOctopus.isLoaded()) {
        // Open "Save Graphiz File" file dialog on button press
        if (ImGui::Button("Save Octopus File")) {
            const char* filters = ".json";
            ImGuiFileDialog::Instance()->OpenDialog("SaveOctopusFileDlgKey", "Save as *.json", filters, data->context.fileDialog.filePath, data->context.fileDialog.fileName);
        }
    }

    // Display "Open Octopus File" file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseOctopusFileDlgKey")) {
        data->context.fileDialog.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        data->context.fileDialog.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

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

    if (data->loadedOctopus.isLoaded()) {
        // Display "Save Octopus File" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveOctopusFileDlgKey")) {
            data->context.fileDialog.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->context.fileDialog.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                const bool isSaved = writeFile(filePathName, data->loadedOctopus.octopusJson);
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
    ImGui::Combo(" ", &data->imageVisualizationParams.selectedDisplayMode, designEditorImageDisplayModes, sizeof(designEditorImageDisplayModes)/sizeof(designEditorImageDisplayModes[0]));

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

    if (ImGui::IsKeyPressed(ImGuiKey_1)) {
        data->context.mode = DesignEditorMode::SELECT;
    } else if (ImGui::IsKeyPressed(ImGuiKey_2)) {
        data->context.mode = DesignEditorMode::ADD_RECTANGLE;
    } else if (ImGui::IsKeyPressed(ImGuiKey_3)) {
        data->context.mode = DesignEditorMode::ADD_ELLIPSE;
    } else if (ImGui::IsKeyPressed(ImGuiKey_4)) {
        data->context.mode = DesignEditorMode::ADD_TEXT;
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        data->context.canvas.zoom = std::clamp(data->context.canvas.zoom + zoomKeySpeed, minZoom, maxZoom);
    } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
        data->context.canvas.zoom = std::clamp(data->context.canvas.zoom - zoomKeySpeed, minZoom, maxZoom);
    }
}
