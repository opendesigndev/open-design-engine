
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
#include <ode/renderer-api.h>
#include "DesignEditorWidgetsContext.h"
#include "DesignEditorLoadedOctopus.h"
#include "DesignEditorImageVisualizationParams.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)

// TODO move to common API utils
static ODE_StringRef stringRef(const std::string &str) {
    ODE_StringRef ref;
    ref.data = str.c_str();
    ref.length = int(str.size());
    return ref;
}

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

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
    glfwFocusWindow(window);
    if (count == 1) {
        DesignEditorWindow &rgiWindow = DesignEditorWindow::getInstance();
        rgiWindow.readOctopusFile(paths[0]);
    }
}

}

struct DesignEditorWindow::Internal {
    Internal(int width_, int height_) :
        gc(GraphicsContext("[Open Design Engine] Design Editor", Vector2i(width_, height_))) {
    }
    ~Internal() {
        loadedOctopus.clear();
    }

    /// Graphics context of the application.
    GraphicsContext gc;
    /// Loaded octopus file data
    // TODO: Loaded Octopus
    DesignEditorLoadedOctopus loadedOctopus;
    /// A flag that is true just after a new Octopus file is loaded
    bool octopusFileReloaded = false;

    /// Current and previous context of the ImGui widgets displayed
    DesignEditorWidgetsContext widgetsContext;
    DesignEditorWidgetsContext prevWidgetsContext;

    /// Current and previous image visualization params
    DesignEditorImageVisualizationParams imageVisualizationParams;
    DesignEditorImageVisualizationParams prevImageVisualizationParams;

    struct Context {
        ODE_EngineHandle engine;
        ODE_EngineAttributes engineAttribs;
        ODE_DesignHandle design;
        ODE_ComponentMetadata metadata = { };
        ODE_RendererContextHandle rc;
        ODE_DesignImageBaseHandle imageBase;
        ODE_PR1_AnimationRendererHandle renderer;
        // Loaded component
        ODE_ComponentHandle component;
    } context;

    struct ZoomContext {
        float designImageZoom = 1.0f;

        const float minZoom = 1.0f;
        const float maxZoom = 10.0f;

        void operator+=(float v) {
            designImageZoom = std::clamp(designImageZoom + v, minZoom, maxZoom);
        }
        void operator-=(float v) {
            *this += -v;
        }
    } zoomContext;

    struct ImGuiWindowContext {
        const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } imGuiWindowContext;

    struct FileDialogContext {
        std::string filePath = ".";
        std::string fileName;
    } fileDialogContext;

    struct Icons {
        TexturePtr cursorTexture = nullptr;
        TexturePtr addRectangleTexture = nullptr;
        TexturePtr addEllipseTexture = nullptr;
        TexturePtr addTextTexture = nullptr;
    } icons;

    enum class Mode {
        SELECT,
        ADD_RECTANGLE,
        ADD_ELLIPSE,
        ADD_TEXT,
    } mode = Mode::SELECT;

    struct LayerPropertiesContext {
        Vector2f translation;
        Vector2f scale;
        float rotation;
        std::string strokeFillText;
        std::vector<std::string> effects;
    } layerPropertiesContext;


    int initialize() {
        CHECK(ode_initializeEngineAttributes(&context.engineAttribs));
        CHECK(ode_createEngine(&context.engine, &context.engineAttribs));
        // TODO: This context is in conflict with the ImGui context.
//        CHECK(ode_createRendererContext(context.engine, &context.rc, stringRef("Design Editor")));
//        CHECK(ode_createDesignImageBase(context.rc, context.design, &context.imageBase));
        return 0;
    }

    int loadOctopus(const std::string &octopusJson) {
        if (context.design.ptr) {
            CHECK(ode_destroyDesign(context.design));
        }

        CHECK(ode_createDesign(context.engine, &context.design));
        CHECK(ode_design_addComponentFromOctopusString(context.design, &context.component, context.metadata, stringRef(octopusJson), nullptr));

        CHECK(ode_component_listLayers(context.component, &loadedOctopus.layerList));

//        ODE_Bitmap bitmap;
//        ODE_PR1_FrameView frameView;
//        CHECK(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &bitmap, &frameView));

        return 0;
    }

    int loadManifest(const std::string &manifestPath) {
        if (context.design.ptr) {
            CHECK(ode_destroyDesign(context.design));
        }

        CHECK(ode_createDesign(context.engine, &context.design));

        const ODE_String manifestPathStr = ode_makeString(manifestPath);
        ODE_ParseError parseError;

        CHECK(ode_design_loadManifestFile(context.design, ode_stringRef(manifestPathStr), &parseError));
        return 0;
    }

    int destroy() {
        CHECK(ode_pr1_destroyAnimationRenderer(context.renderer));
        CHECK(ode_destroyDesignImageBase(context.imageBase));
        CHECK(ode_destroyRendererContext(context.rc));
        CHECK(ode_destroyDesign(context.design));
        CHECK(ode_destroyEngine(context.engine));
        return 0;
    }
};


/*static*/ DesignEditorWindow& DesignEditorWindow::getInstance() {
    static DesignEditorWindow instance(1280, 720);
    return instance;
}

DesignEditorWindow::DesignEditorWindow(int width_, int height_) :
    data(new Internal(width_, height_)) {
    data->initialize();
}

DesignEditorWindow::~DesignEditorWindow() {
    data->destroy();
}

int DesignEditorWindow::display() {
    // Get the GLFW window as initialized in ODE GraphicsContext
    GLFWwindow *window = data->gc.getNativeHandle<GLFWwindow *>();

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

        const bool widgetsContextChanged = data->prevWidgetsContext != data->widgetsContext;
        const bool imageVisualizationParamsChanged = data->imageVisualizationParams != data->prevImageVisualizationParams;

        // Display result texture
        if (data->loadedOctopus.isLoaded()) {
            if (data->octopusFileReloaded || widgetsContextChanged || imageVisualizationParamsChanged) {
                data->octopusFileReloaded = false;

                // TODO: Display result texture
            }
        }

        // ODE DesignEditor controls window
        drawControlsWidget();

        if (data->widgetsContext.showToolbar) {
            drawToolbarWidget();
        }
        if (data->widgetsContext.showLayerList) {
            drawLayerListWidget();
        }
        if (data->widgetsContext.showDesignView) {
            drawDesignViewWidget();
        }
        if (data->widgetsContext.showLayerProperties) {
            drawLayerPropertiesWidget();
        }

        if (data->widgetsContext.showImGuiDebugger) {
            ImGui::ShowMetricsWindow();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const ImVec4 &clearColor = data->imGuiWindowContext.clearColor;
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
    data->octopusFileReloaded = true;
    data->loadedOctopus.clear();

    data->loadedOctopus.filePath = octopusPath;

    if (!readFile(octopusPath, data->loadedOctopus.octopusJson)) {
        fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
        return false;
    }

    const int loadError = data->loadOctopus(data->loadedOctopus.octopusJson);
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
    data->prevWidgetsContext = data->widgetsContext;
    data->prevImageVisualizationParams = data->imageVisualizationParams;

    ImGui::Begin("Controls");

    ImGui::Columns(3);

    // Open "Open Octopus File" file dialog on button press
    if (ImGui::Button("Open Octopus File")) {
        const char* filters = ".json";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Choose Octopus *.json File", filters, data->fileDialogContext.filePath, data->fileDialogContext.fileName);
    }

    if (data->loadedOctopus.isLoaded()) {
        // Open "Save Graphiz File" file dialog on button press
        if (ImGui::Button("Save Octopus File")) {
            const char* filters = ".json";
            ImGuiFileDialog::Instance()->OpenDialog("SaveOctopusFileDlgKey", "Save as *.json", filters, data->fileDialogContext.filePath, data->fileDialogContext.fileName);
        }
    }

    // Display "Open Octopus File" file dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseOctopusFileDlgKey")) {
        data->fileDialogContext.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        data->fileDialogContext.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

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
            data->fileDialogContext.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->fileDialogContext.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

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
    ImGui::Checkbox("Toolbar", &data->widgetsContext.showToolbar);
    ImGui::Checkbox("Layer List", &data->widgetsContext.showLayerList);
    ImGui::Checkbox("Interactive Design View", &data->widgetsContext.showDesignView);
    ImGui::Checkbox("Layer Properties", &data->widgetsContext.showLayerProperties);

    ImGui::NextColumn();

    ImGui::Text("Blend Mode");
    ImGui::Combo(" ", &data->imageVisualizationParams.selectedDisplayMode, designEditorImageDisplayModes, sizeof(designEditorImageDisplayModes)/sizeof(designEditorImageDisplayModes[0]));

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Frame rate:  %f", ImGui::GetIO().Framerate);

    ImGui::NextColumn();

    ImGui::Checkbox("ImGui Metrics/Debugger", &data->widgetsContext.showImGuiDebugger);

    ImGui::End();
}

void DesignEditorWindow::drawToolbarWidget() {
    ImGui::Begin("Toolbar");

    ImGui::End();
}

void drawLayerListRecursiveStep(const ODE_LayerList &layerList, int idx) {
    if (idx >= layerList.n) {
        return;
    }

    const auto areEq = [](const ODE_StringRef &a, const ODE_StringRef &b)->bool {
        return a.length == b.length && strcmp(a.data, b.data) == 0;
    };

    const ODE_LayerList::Entry &rootLayer = layerList.entries[idx];
    const bool hasAnyChildren = (idx+1 < layerList.n) && areEq(layerList.entries[idx+1].parentId, rootLayer.id);

    const std::string layerTypeStr = [](ODE_LayerType layerType) {
        switch (layerType) {
            case ODE_LAYER_TYPE_UNSPECIFIED: return "-";
            case ODE_LAYER_TYPE_SHAPE: return "S";
            case ODE_LAYER_TYPE_TEXT: return "T";
            case ODE_LAYER_TYPE_GROUP: return "G";
            case ODE_LAYER_TYPE_MASK_GROUP: return "M";
            case ODE_LAYER_TYPE_COMPONENT_REFERENCE: return "CR";
            case ODE_LAYER_TYPE_COMPONENT_INSTANCE: return "CI";
        }
        return "-";
    } (rootLayer.type);
    const std::string layerLabel = "["+layerTypeStr+"] "+std::string(rootLayer.name.data);

    if (hasAnyChildren) {
        if (ImGui::TreeNode(layerLabel.c_str())) {
            for (int i = idx+1; i < layerList.n; i++) {
                const ODE_LayerList::Entry &entry = layerList.entries[i];
                if (areEq(entry.parentId, rootLayer.id)) {
                    drawLayerListRecursiveStep(layerList, i);
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::BulletText("%s", layerLabel.c_str());
    }
}

void DesignEditorWindow::drawLayerListWidget() {
    ImGui::Begin("Layer List");

    if (data->loadedOctopus.isLoaded()) {
        drawLayerListRecursiveStep(data->loadedOctopus.layerList, 0);
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void DesignEditorWindow::drawDesignViewWidget() {
    ImGui::Begin("Interactive Design View");

    drawImGuiWidgetTexture(data->icons.cursorTexture->getInternalGLHandle(),
                           data->icons.cursorTexture->dimensions().x,
                           data->icons.cursorTexture->dimensions().y,
                           data->zoomContext.designImageZoom);

    ImGui::End();
}

void DesignEditorWindow::drawLayerPropertiesWidget() {
    ImGui::Begin("Layer Properties");

    ImGui::Text("Translation:");
    ImGui::SameLine(100);
    ImGui::DragFloat2("##translation", &data->layerPropertiesContext.translation.x);

    ImGui::Text("Scale:");
    ImGui::SameLine(100);
    ImGui::DragFloat2("##scale", &data->layerPropertiesContext.scale.x);

    ImGui::Text("Rotation:");
    ImGui::SameLine(100);
    ImGui::DragFloat("##rot", &data->layerPropertiesContext.rotation);

    ImGui::Text("Fill & stroke / text:");
    ImGui::Dummy(ImVec2(20.0f, 0.0f));
    ImGui::SameLine(50);
    ImGui::InputText("##fill", data->layerPropertiesContext.strokeFillText.data(), 50);

    ImGui::Text("Effects:");
    ImGui::SameLine(380);
    if (ImGui::SmallButton("+")) {
        data->layerPropertiesContext.effects.emplace_back();
    }
    int effectToRemove = -1;
    for (size_t ei = 0; ei < data->layerPropertiesContext.effects.size(); ++ei) {
        ImGui::Dummy(ImVec2(20.0f, 0.0f));
        ImGui::SameLine(50);
        ImGui::InputText((std::string("##effect")+std::to_string(ei)).c_str(), data->layerPropertiesContext.effects[ei].data(), 50);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("-##remove")+std::to_string(ei)).c_str())) {
            effectToRemove = static_cast<int>(ei);
        }
    }
    if (effectToRemove >= 0 && effectToRemove < static_cast<int>(data->layerPropertiesContext.effects.size())) {
        data->layerPropertiesContext.effects.erase(data->layerPropertiesContext.effects.begin()+effectToRemove);
    }

    ImGui::End();
}

void DesignEditorWindow::handleKeyboardEvents() {
    const float zoomKeySpeed = 0.03f;

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        data->zoomContext += zoomKeySpeed;
    } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
        data->zoomContext -= zoomKeySpeed;
    }
}
