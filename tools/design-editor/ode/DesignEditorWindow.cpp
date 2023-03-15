
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
#include "DesignEditorRenderer.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)
#define CHECK_IMEND(x) do { if ((x)) { ImGui::End(); return; } } while (false)

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

std::string layerTypeToShortString(ODE_LayerType layerType) {
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
}

std::string layerTypeToString(ODE_LayerType layerType) {
    switch (layerType) {
        case ODE_LAYER_TYPE_UNSPECIFIED: return "-";
        case ODE_LAYER_TYPE_SHAPE: return "Shape";
        case ODE_LAYER_TYPE_TEXT: return "Text";
        case ODE_LAYER_TYPE_GROUP: return "Group";
        case ODE_LAYER_TYPE_MASK_GROUP: return "Mask Group";
        case ODE_LAYER_TYPE_COMPONENT_REFERENCE: return "Component Reference";
        case ODE_LAYER_TYPE_COMPONENT_INSTANCE: return "Component Instance";
    }
    return "-";
}


void drawLayerListRecursiveStep(const ODE_LayerList &layerList, int idx, int &idxClicked) {
    if (idx >= layerList.n) {
        return;
    }

    const auto areEq = [](const ODE_StringRef &a, const ODE_StringRef &b)->bool {
        return a.length == b.length && strcmp(a.data, b.data) == 0;
    };

    const ODE_LayerList::Entry &rootLayer = layerList.entries[idx];
    const bool hasAnyChildren = (idx+1 < layerList.n) && areEq(layerList.entries[idx+1].parentId, rootLayer.id);
    const std::string layerLabel = "["+layerTypeToShortString(rootLayer.type)+"] "+std::string(rootLayer.name.data);

    if (hasAnyChildren) {
        // TODO: Improve these open flags
        if (ImGui::TreeNodeEx((layerLabel+std::string("##")+std::string(rootLayer.id.data)).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                idxClicked = idx;
            }
            for (int i = idx+1; i < layerList.n; i++) {
                const ODE_LayerList::Entry &entry = layerList.entries[i];
                if (areEq(entry.parentId, rootLayer.id)) {
                    drawLayerListRecursiveStep(layerList, i, idxClicked);
                }
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::BulletText("%s", layerLabel.c_str());
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            idxClicked = idx;
        }
    }
}

// TODO: Cleanup:
const char *blendModes[] = {
    "NORMAL",
    "PASS_THROUGH",
    "COLOR",
    "COLOR_BURN",
    "COLOR_DODGE",
    "DARKEN",
    "DARKER_COLOR",
    "DIFFERENCE",
    "DIVIDE",
    "EXCLUSION",
    "HARD_LIGHT",
    "HARD_MIX",
    "HUE",
    "LIGHTEN",
    "LIGHTER_COLOR",
    "LINEAR_BURN",
    "LINEAR_DODGE",
    "LINEAR_LIGHT",
    "LUMINOSITY",
    "MULTIPLY",
    "OVERLAY",
    "PIN_LIGHT",
    "SATURATION",
    "SCREEN",
    "SOFT_LIGHT",
    "SUBTRACT",
    "VIVID_LIGHT",
};

}

struct DesignEditorWindow::Internal {
    Internal() = default;
    ~Internal() {
        loadedOctopus.clear();
    }

    /// Loaded octopus file data
    // TODO: Loaded Octopus
    DesignEditorLoadedOctopus loadedOctopus;
    /// A flag that is true just after a new Octopus file is loaded
    bool octopusFileReloaded = false;

    int selectedLayerId = -1;

    /// Renderer
    std::unique_ptr<DesignEditorRenderer> renderer;

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
        ODE_ComponentHandle component;
        ODE_Bitmap bitmap;
        ODE_PR1_FrameView frameView;
    } context;

    /// Graphics context accessor
    GraphicsContext *gc() {
        return reinterpret_cast<GraphicsContext *>(context.rc.ptr);
    }

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

    struct TexturesContext {
        TexturePtr designImageTexture = nullptr;
    } texturesContext;

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
        CHECK(ode_createRendererContext(context.engine, &context.rc, stringRef("Design Editor")));
        renderer = std::make_unique<DesignEditorRenderer>();
        return 0;
    }

    int loadOctopus(const std::string &octopusJson, const FilePath &octopusPath, const FilePath &fontDir) {
        if (context.design.ptr) {
            CHECK(ode_destroyDesign(context.design));
        }

        CHECK(ode_createDesign(context.engine, &context.design));

        // TODO: Image base:
        CHECK(ode_createDesignImageBase(context.rc, context.design, &context.imageBase));
        reinterpret_cast<ImageBase *>(context.imageBase.ptr)->setImageDirectory(octopusPath.parent());

        CHECK(ode_design_addComponentFromOctopusString(context.design, &context.component, context.metadata, stringRef(octopusJson), nullptr));

        ODE_StringList missingFonts;
        CHECK(ode_design_listMissingFonts(context.design, &missingFonts));

        for (int i = 0; i < missingFonts.n; ++i) {
            if (missingFonts.entries[i].length <= 0) {
                continue;
            }
            const std::string pathStr = (std::string)fontDir+std::string("/")+std::string(missingFonts.entries[i].data)+std::string(".ttf");
            const ODE_StringRef path { pathStr.c_str(), static_cast<int>(strlen(path.data)) };
            ode_design_loadFontFile(context.design, missingFonts.entries[i], path, ODE_StringRef());
        }

        CHECK(ode_component_listLayers(context.component, &loadedOctopus.layerList));

        GLFWwindow *window = gc()->getNativeHandle<GLFWwindow *>();
        glfwGetFramebufferSize(window, &context.frameView.width, &context.frameView.height);
        context.frameView.scale = 1;
        CHECK(ode_pr1_drawComponent(context.rc, context.component, context.imageBase, &context.bitmap, &context.frameView));

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

    const int loadError = data->loadOctopus(data->loadedOctopus.octopusJson, octopusPath, fontDirectory);
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

void DesignEditorWindow::drawLayerListWidget() {
    ImGui::Begin("Layer List");

    if (data->loadedOctopus.isLoaded()) {
        int idxClicked = -1;
        drawLayerListRecursiveStep(data->loadedOctopus.layerList, 0, idxClicked);
        if (idxClicked >= 0) {
            data->selectedLayerId = idxClicked;
        }
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void DesignEditorWindow::drawDesignViewWidget() {
    ImGui::Begin("Interactive Design View");

    const ODE_Bitmap &bmp = data->context.bitmap;
    if (bmp.width > 0 && bmp.height > 0) {
        ode::Bitmap bitmap(PixelFormat::PREMULTIPLIED_RGBA, reinterpret_cast<const void*>(bmp.pixels), bmp.width, bmp.height);

        const ScaledBounds placement {0,0,static_cast<double>(bitmap.width()),static_cast<double>(bitmap.height())};
        data->texturesContext.designImageTexture = data->renderer->blendImageToTexture(std::move(bitmap), placement, 2);

        drawImGuiWidgetTexture(data->texturesContext.designImageTexture->getInternalGLHandle(),
                               data->texturesContext.designImageTexture->dimensions().x,
                               data->texturesContext.designImageTexture->dimensions().y,
                               data->zoomContext.designImageZoom);
    }

    ImGui::End();
}

void DesignEditorWindow::drawLayerPropertiesWidget() {
    ImGui::Begin("Selected Layer Properties");

    if (data->selectedLayerId >= 0 && data->selectedLayerId < data->loadedOctopus.layerList.n) {
        const ODE_LayerList::Entry &selectedLayer = data->loadedOctopus.layerList.entries[data->selectedLayerId];

        bool layerVisible = true; // TODO: Get layer visibility
        float layerOpacity = 1.0f; // TODO: Get layer opacity
        const char *blendModeStr = "NORMAL"; // TODO: Get layer blend mode as string

        ODE_LayerMetrics layerMetrics;
        CHECK_IMEND(ode_component_getLayerMetrics(data->context.component, selectedLayer.id, &layerMetrics));

        const float a = static_cast<float>(layerMetrics.transformation.matrix[0]);
        const float b = static_cast<float>(layerMetrics.transformation.matrix[2]);
        const float c = static_cast<float>(layerMetrics.transformation.matrix[1]);
        const float d = static_cast<float>(layerMetrics.transformation.matrix[3]);
        const float trX = static_cast<float>(layerMetrics.transformation.matrix[4]);
        const float trY = static_cast<float>(layerMetrics.transformation.matrix[5]);

        Vector2f translation {
            trX,
            trY,
        };
        Vector2f scale {
            sqrt(a*a+b*b),
            sqrt(c*c+d*d),
        };
        float rotation = atan(c/d) * (180.0f/M_PI);

        const Vector2f origTranslation = translation;
        const Vector2f origScale = scale;
        const float origRotation = rotation;

        ImGui::Text("%s", "ID:");
        ImGui::SameLine(100);
        ImGui::Text("%s", selectedLayer.id.data);

        ImGui::Text("%s", "Name:");
        ImGui::SameLine(100);
        ImGui::Text("%s", selectedLayer.name.data);

        ImGui::Text("%s", "Type:");
        ImGui::SameLine(100);
        ImGui::Text("%s", layerTypeToString(selectedLayer.type).c_str());

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

        ImGui::Text("Visible:");
        ImGui::SameLine(100);
        if (ImGui::Checkbox("##layer-visibility", &layerVisible)) {
            // TODO: Update layer visiblity
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }

        ImGui::Text("Opacity:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat("##layer-opacity", &layerOpacity)) {
            // TODO: Update layer opacity
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }

        ImGui::Text("Bend mode:");
        ImGui::SameLine(100);
        if (ImGui::BeginCombo("##layer-blend-mode", blendModeStr)) {
            for (int n = 0; n < IM_ARRAYSIZE(blendModes); n++) {
                bool isSelected = (blendModeStr == blendModes[n]);
                if (ImGui::Selectable(blendModes[n], isSelected)) {
                    // TODO: Update layer blend mode
                    blendModeStr = blendModes[n];
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

        ImGui::Text("Translation:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat2("##layer-translation", &translation.x, 1.0f)) {
            const ODE_Transformation newTransformation { 1,0,0,1,translation.x-origTranslation.x,translation.y-origTranslation.y };
            CHECK_IMEND(ode_component_transformLayer(data->context.component, selectedLayer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }

        ImGui::Text("Scale:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat2("##layer-scale", &scale.x, 0.05f, 0.0f, 100.0f)) {
            const ODE_Transformation newTransformation { scale.x/origScale.x,0,0,scale.y/origScale.y,0,0 };
            CHECK_IMEND(ode_component_transformLayer(data->context.component, selectedLayer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }

        ImGui::Text("Rotation:");
        ImGui::SameLine(100);
        if (ImGui::DragFloat("##layer-rotation", &rotation)) {
            const float rotationChangeRad = (rotation-origRotation)*M_PI/180.0f;
            const ODE_Transformation newTransformation { cos(rotationChangeRad),-sin(rotationChangeRad),sin(rotationChangeRad),cos(rotationChangeRad),0,0 };
            CHECK_IMEND(ode_component_transformLayer(data->context.component, selectedLayer.id, ODE_TRANSFORMATION_BASIS_LAYER, newTransformation));
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

        ImGui::Text("Fill & stroke / text:");
        ImGui::Dummy(ImVec2(20.0f, 0.0f));
        ImGui::SameLine(50);
        ImGui::InputText("##layer-fill", data->layerPropertiesContext.strokeFillText.data(), 50);

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

        ImGui::Text("Effects:");
        ImGui::SameLine(415);
        if (ImGui::SmallButton("+##layer-effect-add")) {
            data->layerPropertiesContext.effects.emplace_back();
        }
        int effectToRemove = -1;
        for (size_t ei = 0; ei < data->layerPropertiesContext.effects.size(); ++ei) {
            ImGui::Dummy(ImVec2(20.0f, 0.0f));
            ImGui::SameLine(50);
            ImGui::InputText((std::string("##layer-effect-")+std::to_string(ei)).c_str(), data->layerPropertiesContext.effects[ei].data(), 50);
            ImGui::SameLine(415);
            if (ImGui::SmallButton((std::string("-##layer-effect-remove")+std::to_string(ei)).c_str())) {
                effectToRemove = static_cast<int>(ei);
            }
        }
        if (effectToRemove >= 0 && effectToRemove < static_cast<int>(data->layerPropertiesContext.effects.size())) {
            data->layerPropertiesContext.effects.erase(data->layerPropertiesContext.effects.begin()+effectToRemove);
        }

        ImGui::Dummy(ImVec2 { 0.0f, 10.0f });

        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImVec4 { 0.6f, 0.0f, 0.0f, 1.0f }));
        ImGui::SameLine(100);
        if (ImGui::Button("Delete Layer [FUTURE_API]##layer-delete", ImVec2 { 250, 20 })) {
            // TODO: Remove layer when API available
            // CHECK_IMEND(ode_component_removeLayer(data->context.component, selectedLayer.id));
            CHECK_IMEND(ode_pr1_drawComponent(data->context.rc, data->context.component, data->context.imageBase, &data->context.bitmap, &data->context.frameView));
        }
        ImGui::PopStyleColor(1);
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
