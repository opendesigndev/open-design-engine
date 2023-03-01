
#include "DesignEditorWindow.h"

#include <queue>

// ImGui includes
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// ImGuiFileDialog includes
#include <ImGuiFileDialog.h>

// OD includes
#include <ode-media.h>
#include "DesignEditorRenderer.h"
#include "DesignEditorOctopusLoader.h"
#include "DesignEditorWidgetsContext.h"
#include "DesignEditorLoadedOctopus.h"
#include "DesignEditorImageVisualizationParams.h"

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
    /// The rendered image of the root node in graph
    PlacedImagePtr rootNodeImage;
    /// Loaded octopus file data
    DesignEditorLoadedOctopus loadedOctopus;
    /// A flag that is true just after a new Octopus file is loaded
    bool octopusFileReloaded = false;

    /// Renderer
    DesignEditorRenderer renderer;

    /// Current and previous context of the ImGui widgets displayed
    DesignEditorWidgetsContext widgetsContext;
    DesignEditorWidgetsContext prevWidgetsContext;

    /// Current and previous image visualization params
    DesignEditorImageVisualizationParams imageVisualizationParams;
    DesignEditorImageVisualizationParams prevImageVisualizationParams;

    struct ZoomContext {
        float resultImageZoom = 1.0f;

        const float minZoom = 1.0f;
        const float maxZoom = 10.0f;

        void operator+=(float v) {
            resultImageZoom = std::clamp(resultImageZoom + v, minZoom, maxZoom);
        }
        void operator-=(float v) {
            *this += -v;
        }
    } zoomContext;

    struct TexturesContext {
        TexturePtr resultTexture = nullptr;
    } texturesContext;

    struct ImGuiWindowContext {
        const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } imGuiWindowContext;

    struct FileDialogContext {
        std::string filePath = ".";
        std::string fileName;
    } fileDialogContext;
};


/*static*/ DesignEditorWindow& DesignEditorWindow::getInstance() {
    static DesignEditorWindow instance(1280, 720);
    return instance;
}

DesignEditorWindow::DesignEditorWindow(int width_, int height_) :
    data(new Internal(width_, height_)) {
}

DesignEditorWindow::~DesignEditorWindow() {
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

                data->texturesContext.resultTexture.reset();

                const BitmapPtr bitmap = data->loadedOctopus.resultBitmap();
                const std::optional<ode::ScaledBounds> placementOpt = data->loadedOctopus.resultPlacement();
                if (bitmap && placementOpt.has_value()) {
                    // TODO: Image visualization params
                    data->texturesContext.resultTexture = data->renderer.blendImageToTexture(bitmap, placementOpt.value(), data->imageVisualizationParams.selectedDisplayMode);
                }
            }
        }

        // ODE DesignEditor controls window
        drawControlsWidget();

        if (data->widgetsContext.showResultImage) {
            drawResultImageWidget(data->zoomContext.resultImageZoom);
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

bool DesignEditorWindow::readOctopusFile(const FilePath &octopusPath) {
    data->octopusFileReloaded = true;
    data->loadedOctopus.clear();

    const DesignEditorOctopusLoader::Input loaderInput { octopusPath, imageDirectory, fontDirectory, ignoreValidation };
    const bool isLoaded = DesignEditorOctopusLoader::readOctopusFile(loaderInput, data->gc, data->loadedOctopus);
    if (!isLoaded) {
        data->loadedOctopus.clear();
    }

    return isLoaded;
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

    ImGui::NextColumn();

    ImGui::Text("Widgets:");
    ImGui::Checkbox("Result image", &data->widgetsContext.showResultImage);

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

void DesignEditorWindow::drawResultImageWidget(float &zoom) {
    ImGui::Begin("Result image");

    if (data->texturesContext.resultTexture != nullptr) {
        const BitmapPtr &bitmap = data->loadedOctopus.bitmap;
        drawImGuiWidgetTexture(data->texturesContext.resultTexture->getInternalGLHandle(), bitmap->width(), bitmap->height(), zoom, 1);
    } else {
        ImGui::Text("---");
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
