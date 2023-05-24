
#include "RGIWindow.h"

#include <queue>

// ImGui includes
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// ImGuiFileDialog includes
#include <ImGuiFileDialog.h>

// Imnodes includes
#include <imnodes.h>

// OD includes
#include <ode-media.h>
#include "RGIRenderer.h"
#include "RGIRenderedNode.h"
#include "ImnodesHelpers.h"
#include "RGIOctopusLoader.h"
#include "RGIWidgetsContext.h"
#include "RGILoadedOctopus.h"
#include "RGIImageVisualizationParams.h"
#include "RGINodeGraphContext.h"

namespace {

void drawImGuiWidgetTexture(const GLuint textureHandle, int width, int height, float &zoom, size_t colsCount = 1, size_t rowsCount = 1) {
    const ImVec2 windowSize = ImGui::GetWindowSize();

    const int horizontalPadding = 18;
    const int verticalPadding = 100;
    const float scaling = std::min(
        static_cast<float>(windowSize.x / static_cast<float>(colsCount) - horizontalPadding) / static_cast<float>(width),
        static_cast<float>(windowSize.y / static_cast<float>(rowsCount) - verticalPadding) / static_cast<float>(height));

    const ImVec2 newImageSize(std::max(scaling * width, 0.0f) * zoom, std::max(scaling * height, 0.0f) * zoom);

#ifdef ODE_DEBUG
    ImGui::Text("GL Handle:        %d", textureHandle);
#endif
    ImGui::Text("Texture size:     %d x %d", width, height);
    ImGui::Text("Display size:     %d x %d", static_cast<int>(std::round(newImageSize.x)), static_cast<int>(std::round(newImageSize.y)));
    ImGui::SliderFloat("Zoom [-S][+W]", &zoom, 1.0f, 10.0f);

    ImGui::Image((void*)(intptr_t)textureHandle, newImageSize);
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
    glfwFocusWindow(window);
    if (count == 1) {
        RGIWindow &rgiWindow = RGIWindow::getInstance();
        rgiWindow.readOctopusFile(paths[0]);
    }
}

void miniMapHoveringCallback(int nodeId, void* /*userData*/) {
    ImGui::SetTooltip("#%d", nodeId);
}

}

struct RGIWindow::Internal {
    Internal(int width_, int height_) :
        gc(GraphicsContext("[Open Design Engine] Render Graph Inspector", Vector2i(width_, height_))) {
    }
    ~Internal() {
        loadedOctopus.clear();
    }

    /// Graphics context of the application.
    GraphicsContext gc;
    /// Loaded octopus file data
    RGILoadedOctopus loadedOctopus;
    /// A flag that is true just after a new Octopus file is loaded
    bool octopusFileReloaded = false;

    RGIRenderer renderer;

    /// Current and previous context of the ImGui widgets displayed
    RGIWidgetsContext widgetsContext;
    RGIWidgetsContext prevWidgetsContext;

    /// Current and previous context of the calculated node graph
    RGINodeGraphContext nodeGraphContext;
    RGINodeGraphContext prevNodeGraphContext;

    /// Current and previous image visualization params
    RGIImageVisualizationParams imageVisualizationParams;
    RGIImageVisualizationParams prevImageVisualizationParams;

    /// Current and previous image comparison params
    RGIImageComparisonParams imageComparisonParams;
    RGIImageComparisonParams prevImageComaprisonParams;

    /// Current and previous image version comparison params
    RGIImageComparisonParams imageVersionComparisonParams;
    RGIImageComparisonParams prevImageVersionComaprisonParams;

    struct ZoomContext {
        float resultImageZoom = 1.0f;
        float selectedImagesZoom = 1.0f;
        float imageComparisonZoom = 1.0f;
        float imageVersionsComparisonZoom = 1.0f;

        const float minZoom = 1.0f;
        const float maxZoom = 10.0f;

        void operator+=(float v) {
            resultImageZoom = std::clamp(resultImageZoom + v, minZoom, maxZoom);
            selectedImagesZoom = std::clamp(selectedImagesZoom + v, minZoom, maxZoom);
            imageComparisonZoom = std::clamp(imageComparisonZoom + v, minZoom, maxZoom);
            imageVersionsComparisonZoom = std::clamp(imageVersionsComparisonZoom + v, minZoom, maxZoom);
        }
        void operator-=(float v) {
            *this += -v;
        }
    } zoomContext;

    struct TexturesContext {
        TexturePtr resultTexture = nullptr;
        std::vector<TexturePtr> selectedTextures;
        TexturePtr compareTexture = nullptr;
        TexturePtr compareTextures[2] { nullptr };
        TexturePtr versionsComparisonTexture = nullptr;
        TexturePtr versionsComparisonTextures[2] { nullptr, nullptr };
    } texturesContext;

    struct ImGuiWindowContext {
        const ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    } imGuiWindowContext;

    struct FileDialogContext {
        std::string filePath = ".";
        std::string fileName;
    } fileDialogContext;

    struct NodeGraphSettings {
        const float spacing = 200.0f;
        const float defaultYTopPadding = 20.0f;
        const float defaultXStartPos = 1000.0f;
        const ImU32 leafColor = IM_COL32(200, 109, 191, 255);
        const ImU32 nodeColor = IM_COL32(11, 109, 191, 255);
        const ImU32 leafColorSelected = IM_COL32(81, 148, 204, 255);
        const ImU32 nodeColorSelected = IM_COL32(10, 148, 204, 255);
    } nodeGraphSettings;

    void updateSelectedNodes() {
        // Keep the selection if initial selection
        if (nodeGraphContext.isInitialSelection()) {
            return;
        }

        nodeGraphContext.selectedNodes.clear();

        // Get selected node ids - either from root nodes found using the textfield or by mouse selection
        if (nodeGraphContext.isLayerSearchSelection() && strlen(nodeGraphContext.layerSelectionStr) != 0) {
            ImNodes::ClearNodeSelection();
            for (const RGIRenderedNode &node : loadedOctopus.renderedNodes) {
                const octopus::Layer *layer = node.node->getLayer();
                if (layer != nullptr && ((!layer->id.empty() && layer->id.find(nodeGraphContext.layerSelectionStr) != std::string::npos) || (!layer->name.empty() && layer->name.find(nodeGraphContext.layerSelectionStr) != std::string::npos))) {
                    nodeGraphContext.selectedNodes.emplace_back(node.index);
                }
            }

        } else if (nodeGraphContext.isNodesSearchSelection() && strlen(nodeGraphContext.nodesSelectionStr) != 0) {
            ImNodes::ClearNodeSelection();
            const std::string &selectionString = nodeGraphContext.nodesSelectionStr;
            if (selectionString.front() == '#') {
                if (selectionString.size() > 1 && selectionString.size() <= 7) {
                    const std::string selectionStringNumber = std::string(selectionString.begin() + 1, selectionString.end());
                    const bool isNumber = (selectionStringNumber.find_first_not_of( "0123456789" ) == std::string::npos);
                    if (isNumber) {
                        const int searchIndex = std::stoi(selectionStringNumber);
                        if (searchIndex > 0 && searchIndex < static_cast<int>(loadedOctopus.renderedNodes.size())) {
                            nodeGraphContext.selectedNodes.emplace_back(searchIndex);
                        }
                    }
                }
            } else {
                for (const RGIRenderedNode &node : loadedOctopus.renderedNodes) {
                    if (node.name.find(nodeGraphContext.nodesSelectionStr) != std::string::npos) {
                        nodeGraphContext.selectedNodes.emplace_back(node.index);
                    }
                }
            }

        } else {
            const int numSelectedNodes = ImNodes::NumSelectedNodes();
            if (numSelectedNodes > 0) {
                nodeGraphContext.selectedNodes.resize(numSelectedNodes);
                ImNodes::GetSelectedNodes(nodeGraphContext.selectedNodes.data());
            }
        }
    }

    RGIRenderedNodes getSelectedNodes() const {
        RGIRenderedNodes selectedNodes;

        if (!nodeGraphContext.selectedNodes.empty()) {
            for (const RGIRenderedNode &renderedNode : loadedOctopus.renderedNodes) {
                if (std::find(nodeGraphContext.selectedNodes.begin(), nodeGraphContext.selectedNodes.end(), renderedNode.index) != nodeGraphContext.selectedNodes.end()) {
                    selectedNodes.emplace_back(renderedNode);
                }
            }
        }

        return selectedNodes;
    }

    RGIRenderedNodes getVersionCompareSelectedNodes() const {
        RGIRenderedNodes selectedNodes;

        if (!nodeGraphContext.selectedNodes.empty()) {
            for (const RGIRenderedNode &renderedNode : loadedOctopus.renderedNodes) {
                if (std::find(nodeGraphContext.selectedNodes.begin(), nodeGraphContext.selectedNodes.end(), renderedNode.index) != nodeGraphContext.selectedNodes.end()) {
                    selectedNodes.emplace_back(renderedNode);

                    if (nodeGraphContext.selectedNodes.size() == 1) {
                        break;
                    }
                }
            }
            for (const RGIRenderedNode &renderedNode : loadedOctopus.renderedNodesAltVersion) {
                if (std::find(nodeGraphContext.selectedNodes.begin(), nodeGraphContext.selectedNodes.end(), renderedNode.index) != nodeGraphContext.selectedNodes.end()) {
                    selectedNodes.emplace_back(renderedNode);

                    if (nodeGraphContext.selectedNodes.size() == 1) {
                        break;
                    }
                }
            }
        }

        return selectedNodes;
    }
};


/*static*/ RGIWindow& RGIWindow::getInstance() {
    static RGIWindow instance(1280, 720);
    return instance;
}

RGIWindow::RGIWindow(int width_, int height_) :
    data(new Internal(width_, height_)) {
}

RGIWindow::~RGIWindow() {
}

int RGIWindow::display() {
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

    // Setup Imnodes context
    ImNodes::CreateContext();

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

        const RGIRenderedNodes selectedNodes = data->getSelectedNodes();
        const RGIRenderedNodes versionCompareSelectedNodes = data->getVersionCompareSelectedNodes();
        const bool widgetsContextChanged = data->prevWidgetsContext != data->widgetsContext;
        const bool nodeGraphChanged = data->nodeGraphContext != data->prevNodeGraphContext;
        const bool imageVisualizationParamsChanged = data->imageVisualizationParams != data->prevImageVisualizationParams;
        const bool imageComparisonParamsChanged = data->imageComparisonParams != data->prevImageComaprisonParams;
//        const bool imageVersionComparisonParamsChanged = data->imageVersionComparisonParams != data->prevImageVersionComaprisonParams;

        // Display result texture
        if (data->loadedOctopus.isLoaded()) {
            if (data->octopusFileReloaded) {
                data->octopusFileReloaded = false;

                data->texturesContext.resultTexture.reset();

                const BitmapPtr bitmap = data->loadedOctopus.resultBitmap();
                const std::optional<ode::ScaledBounds> placementOpt = data->loadedOctopus.resultPlacement();
                if (bitmap && placementOpt.has_value()) {
                    data->texturesContext.resultTexture = data->renderer.blendImageToTexture(bitmap, placementOpt.value(), data->imageVisualizationParams.selectedDisplayMode);
                }
            }
        }

        // ODE RGI controls window
        drawControlsWidget();

        if (data->widgetsContext.showResultImage) {
            drawResultImageWidget(data->zoomContext.resultImageZoom);
        }
        if (data->widgetsContext.showRenderGraph) {
            drawRenderGraphWidget();
        }
        if (data->widgetsContext.showRenderGraphInfo) {
            drawRenderGraphInfoWidget();
        }

        // Update selected nodes, only after the ImNodes render graph has been generated
        data->updateSelectedNodes();

        // Display selected texture
        if (!selectedNodes.empty()) {
            if (nodeGraphChanged || widgetsContextChanged || imageVisualizationParamsChanged) {
                if (data->widgetsContext.showSelectedImages) {
                    data->texturesContext.selectedTextures.clear();
                    data->texturesContext.selectedTextures.resize(selectedNodes.size());

                    for (size_t i = 0; i < selectedNodes.size(); ++i) {
                        const BitmapPtr &bitmap = selectedNodes[i].bitmap;
                        if (bitmap) {
                            const ScaledBounds &placement = selectedNodes[i].placement;
                            data->texturesContext.selectedTextures[i] = data->renderer.blendImageToTexture(bitmap, placement, data->imageVisualizationParams.selectedDisplayMode);
                        }
                    }
                }
            }

            if (nodeGraphChanged || widgetsContextChanged || imageVisualizationParamsChanged || imageComparisonParamsChanged) {
                if (data->widgetsContext.showImageComparison && selectedNodes.size() == 2) {
                    data->texturesContext.compareTexture.reset();
                    data->texturesContext.compareTextures[0].reset();
                    data->texturesContext.compareTextures[1].reset();

                    const BitmapPtr &bitmapL = selectedNodes.front().bitmap;
                    const BitmapPtr &bitmapR = selectedNodes.back().bitmap;
                    const ScaledBounds &placementL = selectedNodes.front().placement;
                    const ScaledBounds &placementR = selectedNodes.back().placement;

                    if (bitmapL && bitmapR) {
                        data->texturesContext.compareTexture = data->renderer.compareImagesToTexture(bitmapL, bitmapR, placementL, placementR, data->imageVisualizationParams.selectedDisplayMode, data->imageComparisonParams);
                        data->texturesContext.compareTextures[0] = data->renderer.blendImageToTexture(bitmapL, placementL, data->imageVisualizationParams.selectedDisplayMode);
                        data->texturesContext.compareTextures[1] = data->renderer.blendImageToTexture(bitmapR, placementR, data->imageVisualizationParams.selectedDisplayMode);
                    }
                }
            }
        }
        // Version comparison
        if (!versionCompareSelectedNodes.empty()) {
            // TODO: Matus: Fix this comparison condition
//            if (nodeGraphChanged || widgetsContextChanged || imageVisualizationParamsChanged || imageVersionComparisonParamsChanged) {
                if (data->widgetsContext.showVersionsComparison && versionCompareSelectedNodes.size() == 2) {
                    data->texturesContext.versionsComparisonTexture.reset();
                    data->texturesContext.versionsComparisonTextures[0].reset();
                    data->texturesContext.versionsComparisonTextures[1].reset();

                    const BitmapPtr &bitmapL = versionCompareSelectedNodes.front().bitmap;
                    const BitmapPtr &bitmapR = versionCompareSelectedNodes.back().bitmap;
                    const ScaledBounds &placementL = versionCompareSelectedNodes.front().placement;
                    const ScaledBounds &placementR = versionCompareSelectedNodes.back().placement;

                    if (bitmapL && bitmapR) {
                        data->texturesContext.versionsComparisonTexture = data->renderer.compareImagesToTexture(bitmapL, bitmapR, placementL, placementR, data->imageVisualizationParams.selectedDisplayMode, data->imageVersionComparisonParams);
                        data->texturesContext.versionsComparisonTextures[0] = data->renderer.blendImageToTexture(bitmapL, placementL, data->imageVisualizationParams.selectedDisplayMode);
                        data->texturesContext.versionsComparisonTextures[1] = data->renderer.blendImageToTexture(bitmapR, placementR, data->imageVisualizationParams.selectedDisplayMode);
                    }
                }
//            }
        }

        if (data->widgetsContext.showSelectedImages) {
            drawSelectedImagesWidget(data->zoomContext.selectedImagesZoom);
        }
        if (data->widgetsContext.showImageComparison) {
            drawImageComparisonWidget(data->zoomContext.imageComparisonZoom);
        }
        if (data->widgetsContext.showImGuiDebugger) {
            ImGui::ShowMetricsWindow();
        }
        if (data->widgetsContext.showVersionsComparison) {
            drawVersionsComparisonWidget(data->zoomContext.imageVersionsComparisonZoom);
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

    // Cleanup Imnodes
    ImNodes::DestroyContext();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

bool RGIWindow::readOctopusFile(const FilePath &octopusPath) {
    data->octopusFileReloaded = true;
    data->loadedOctopus.clear();
    data->nodeGraphContext.clear();

    const RGIOctopusLoader::Input loaderInput { octopusPath, imageDirectory, fontDirectory, ignoreValidation };
    const bool isLoaded = RGIOctopusLoader::readOctopusFile(loaderInput, data->gc, data->loadedOctopus);

    if (!isLoaded) {
        data->loadedOctopus.clear();
    } else {
        data->nodeGraphContext.selectionMode = RGINodeGraphContext::SelectionMode::INIT_ROOT_NODE;
        data->nodeGraphContext.selectedNodes = { 1 };
    }

    return isLoaded;
}

void RGIWindow::setImageDirectory(const FilePath &imageDirectory_) {
    imageDirectory = imageDirectory_;
}

void RGIWindow::setFontDirectory(const FilePath &fontDirectory_) {
    fontDirectory = fontDirectory_;
}

void RGIWindow::setIgnoreValidation(bool ignoreValidation_) {
    ignoreValidation = ignoreValidation_;
}

void RGIWindow::drawControlsWidget() {
    data->prevWidgetsContext = data->widgetsContext;
    data->prevImageVisualizationParams = data->imageVisualizationParams;

    ImGui::Begin("Controls");

    ImGui::Columns(3);

    // Open "Open Octopus File" file dialog on button press
    if (ImGui::Button("Open Octopus File")) {
        const char* filters = ".json";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseOctopusFileDlgKey", "Choose Octopus *.json File", filters, data->fileDialogContext.filePath, data->fileDialogContext.fileName);
    }

    if (!data->loadedOctopus.graphVizStr.empty()) {
        // Open "Save Graphiz File" file dialog on button press
        if (ImGui::Button("Save Graphviz File")) {
            const char* filters = ".gv";
            ImGuiFileDialog::Instance()->OpenDialog("SaveGraphvizFileDlgKey", "Save as *.gv", filters, data->fileDialogContext.filePath, data->fileDialogContext.fileName);
        }
    }

    if (data->nodeGraphContext.selectedNodes.size() == 1) {
        // Open "Save Intermediate Image" file dialog on button press
        if (ImGui::Button("Save image")) {
            const char* filters = ".png";
            ImGuiFileDialog::Instance()->OpenDialog("SaveAsPngFileDlgKey", "Save as *.png", filters, data->fileDialogContext.filePath, data->fileDialogContext.fileName);
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

            data->nodeGraphContext.intialized = false;
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (!data->loadedOctopus.graphVizStr.empty()) {
        // Display "Save Graphiz File" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveGraphvizFileDlgKey")) {
            data->fileDialogContext.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->fileDialogContext.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                const bool isSaved = writeFile(filePathName, data->loadedOctopus.graphVizStr);
                if (!isSaved) {
                    fprintf(stderr, "Internal error (saving render graph visualization to filesystem)\n");
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    if (data->nodeGraphContext.selectedNodes.size() == 1) {
        // Display "Save Intermediate Image" file dialog
        if (ImGuiFileDialog::Instance()->Display("SaveAsPngFileDlgKey")) {
            data->fileDialogContext.filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            data->fileDialogContext.fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            if (ImGuiFileDialog::Instance()->IsOk()) {
                for (const RGIRenderedNode &renderedNode : data->loadedOctopus.renderedNodes) {
                    if (renderedNode.index == data->nodeGraphContext.selectedNodes.front()) {
                        const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

                        const bool isSaved = savePng(filePathName, *renderedNode.bitmap);
                        if (!isSaved) {
                            fprintf(stderr, "Internal error (saving image to png)\n");
                        }
                    }
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }
    }

    ImGui::NextColumn();

    ImGui::Text("Widgets:");
    ImGui::Checkbox("Result image", &data->widgetsContext.showResultImage);
    ImGui::Checkbox("Render graph", &data->widgetsContext.showRenderGraph);
    ImGui::Checkbox("Render graph stats", &data->widgetsContext.showRenderGraphInfo);
    ImGui::Checkbox("Selected images", &data->widgetsContext.showSelectedImages);
    ImGui::Checkbox("Image comparison", &data->widgetsContext.showImageComparison);

    ImGui::NextColumn();

    ImGui::Text("Blend Mode");
    ImGui::Combo(" ", &data->imageVisualizationParams.selectedDisplayMode, rgiImageDisplayModes, sizeof(rgiImageDisplayModes)/sizeof(rgiImageDisplayModes[0]));

    char prevLayerSelectionStr[sizeof(data->nodeGraphContext.layerSelectionStr)];
    strcpy(prevLayerSelectionStr, data->nodeGraphContext.layerSelectionStr);

    ImGui::Text("Select Layer");
    ImGui::InputText("search layers", data->nodeGraphContext.layerSelectionStr, sizeof(data->nodeGraphContext.layerSelectionStr));

    char prevNodesSelectionStr[sizeof(data->nodeGraphContext.nodesSelectionStr)];
    strcpy(prevNodesSelectionStr, data->nodeGraphContext.nodesSelectionStr);

    ImGui::Text("Select Nodes");
    ImGui::InputText("search nodes", data->nodeGraphContext.nodesSelectionStr, sizeof(data->nodeGraphContext.nodesSelectionStr));

    // Change selection mode to SEARCH_LAYER or SEARCH_NODES when textfield value changes
    if (!data->nodeGraphContext.isLayerSearchSelection() && strcmp(prevLayerSelectionStr, data->nodeGraphContext.layerSelectionStr) != 0) {
        data->nodeGraphContext.selectionMode = RGINodeGraphContext::SelectionMode::SEARCH_LAYER;
    }
    if (!data->nodeGraphContext.isNodesSearchSelection() && strcmp(prevNodesSelectionStr, data->nodeGraphContext.nodesSelectionStr) != 0) {
        data->nodeGraphContext.selectionMode = RGINodeGraphContext::SelectionMode::SEARCH_NODES;
    }

    ImGui::NextColumn();
    ImGui::Separator();
    ImGui::Text("Frame rate:  %f", ImGui::GetIO().Framerate);

    ImGui::NextColumn();

    ImGui::Checkbox("ImGui Metrics/Debugger", &data->widgetsContext.showImGuiDebugger);

    ImGui::Separator();
    ImGui::Checkbox("Versions compare", &data->widgetsContext.showVersionsComparison);

    ImGui::End();
}

void RGIWindow::drawRenderGraphWidget() {
    data->prevNodeGraphContext = data->nodeGraphContext;

    ImGui::Begin("Render Graph");

    const RGIRenderGraph &renderGraph = data->loadedOctopus.renderGraph;

    if (renderGraph.empty()) {
        ImGui::End();
        return;
    }

    ImNodes::BeginNodeEditor();

    int nodeIndex = 0;
    int inAttrIndex = static_cast<int>(renderGraph.getNodesCount());

    std::vector<size_t> nodeLevelPositions;
    nodeLevelPositions.resize(renderGraph.getMaxNodeDepth() + 1);

    std::vector<std::pair<int, int>> links;

    std::set<const Rendexpr *> visited;
    std::queue<const Rendexpr *> queue;
    queue.push(renderGraph.getRoot().get());

    while (!queue.empty()) {
        const Rendexpr *node = queue.front();
        queue.pop();
        if (!(node && visited.insert(node).second)) {
            continue;
        }

        // Set the titlebar color of an individual node
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, renderGraph.getNodesChildrenCount(node) > 0 ? data->nodeGraphSettings.nodeColor : data->nodeGraphSettings.leafColor);
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, renderGraph.getNodesChildrenCount(node) > 0 ? data->nodeGraphSettings.nodeColorSelected : data->nodeGraphSettings.leafColorSelected);

        ImNodes::BeginNode(++nodeIndex);

        if (!data->nodeGraphContext.intialized) {
            const int nodeDepth = renderGraph.getNodeDepth(node);

            if (nodeDepth >= 0) {
                const float xPos = data->nodeGraphSettings.defaultXStartPos - static_cast<float>(nodeDepth) * data->nodeGraphSettings.spacing;
                const float yPos = static_cast<float>(nodeLevelPositions[nodeDepth]) * data->nodeGraphSettings.spacing + data->nodeGraphSettings.defaultYTopPadding;

                ImNodes::SetNodeEditorSpacePos(nodeIndex, ImVec2(xPos, yPos));
                ImNodes::SetNodeDraggable(nodeIndex, true);

                ++nodeLevelPositions[nodeDepth];
            }
        }

        switch (node->type) {
            #define VISIT_NODE(T) \
                AddNodeTitleBar(renderExpressionTypeShortName(node->type)+NodeEnd(nodeIndex))

            #define VISIT_CHILD(T, m) { \
                AddInputAttribute(++inAttrIndex, #m); \
                queue.push(static_cast<const T *>(node)->m.get()); \
                const int outAttrIndex = data->loadedOctopus.renderGraph.getNodeIndex(static_cast<const T *>(node)->m.get())+1; \
                if (outAttrIndex > 0) { links.emplace_back(std::make_pair(outAttrIndex, inAttrIndex)); } \
            }

            RENDER_EXPRESSION_CASES(VISIT_NODE, VISIT_CHILD)
        }

        const int outAttrIndex = nodeIndex;
        AddOutputAttribute(outAttrIndex, "-> output");

        ImNodes::EndNode();

        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    // Add node links
    for (int i = 0; i < static_cast<int>(links.size()); ++i) {
        const std::pair<int, int> &p = links[i];
        ImNodes::Link(i, p.first, p.second);
    }

    // Manually select nodes that are selected via one of the text search boxes or keyboard
    if ((data->nodeGraphContext.isInitialSelection()) ||
        (data->nodeGraphContext.isLayerSearchSelection() && strlen(data->nodeGraphContext.layerSelectionStr) != 0) ||
        (data->nodeGraphContext.isNodesSearchSelection() && strlen(data->nodeGraphContext.nodesSelectionStr) != 0)) {
        ImNodes::ClearNodeSelection();
        for (const int &selectedNode : data->nodeGraphContext.selectedNodes) {
            if (!ImNodes::IsNodeSelected(selectedNode)) {
                ImNodes::SelectNode(selectedNode);
            }
        }
    }

    // Add minimap
    ImNodes::MiniMap(0.3f, ImNodesMiniMapLocation_BottomLeft, miniMapHoveringCallback);

    ImNodes::EndNodeEditor();

    // If mouse clicked in textfield selection mode over the render graph widget, switch to mouse selection mode
    const bool isMouseClicked = ImGui::IsMouseClicked(0);
    if (isMouseClicked && !data->nodeGraphContext.isMouseSelection()) {
        const ImVec2 windowPos = ImGui::GetWindowPos();
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const ImVec2 clickPos = ImGui::GetMousePos();
        const ImVec2 clickPosInWindow = ImVec2(clickPos.x - windowPos.x, clickPos.y - windowPos.y);

        const bool isRGWidgetClicked = clickPosInWindow.x > 0.0 && clickPosInWindow.y > 0.0 && clickPosInWindow.x < windowSize.x && clickPosInWindow.y < windowSize.y;
        if (isRGWidgetClicked) {
            data->nodeGraphContext.selectionMode = RGINodeGraphContext::SelectionMode::MOUSE;
        }
    }

    ImGui::End();

    data->nodeGraphContext.intialized = true;
}

void RGIWindow::drawRenderGraphInfoWidget() {
    ImGui::Begin("Render graph info");

    const RGIRenderGraph &renderGraph = data->loadedOctopus.renderGraph;

    ImGui::Text("Nodes count:      %u", static_cast<unsigned int>(renderGraph.getNodesCount()));
    ImGui::Text("Max node depth:   %u", static_cast<unsigned int>(renderGraph.getMaxNodeDepth()));

    ImGui::Separator();

    ImGui::Text("Layers:");

    for (const octopus::Layer *layer : data->loadedOctopus.layers) {
        ImGui::Text("  %s | %s", layer->id.c_str(), layer->name.c_str());
    }

    ImGui::End();
}

void RGIWindow::drawResultImageWidget(float &zoom) {
    ImGui::Begin("Result image");

    const RGIRenderedNode *resultNode = data->loadedOctopus.resultNode();
    if (data->loadedOctopus.resultNode() != nullptr && data->texturesContext.resultTexture != nullptr) {
        const BitmapPtr &bitmap = resultNode->bitmap;

        ImGui::Text("%s", resultNode->name.c_str());
        drawImGuiWidgetTexture(data->texturesContext.resultTexture->getInternalGLHandle(), bitmap->width(), bitmap->height(), zoom, 1);
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void RGIWindow::drawSelectedImagesWidget(float &zoom) {
    ImGui::Begin("Selected images");

    const RGIRenderedNodes selectedNodes = data->getSelectedNodes();

    const bool allNodesHaveBitmap = std::all_of(selectedNodes.begin(), selectedNodes.end(), [](const RGIRenderedNode &selectedNode)->bool {
        return selectedNode.bitmap != nullptr;
    });
    if (!selectedNodes.empty() && allNodesHaveBitmap && selectedNodes.size() == data->texturesContext.selectedTextures.size()) {
        for (size_t i = 0; i < selectedNodes.size(); ++i) {
            if (data->texturesContext.selectedTextures[i] != nullptr) {
                const GLuint handle = data->texturesContext.selectedTextures[i]->getInternalGLHandle();
                const BitmapPtr &bitmap = selectedNodes[i].bitmap;

                if (i == 0) {
                    ImGui::Columns(static_cast<int>(selectedNodes.size()));
                } else {
                    ImGui::NextColumn();
                }

                ImGui::Text("%s", selectedNodes[i].name.c_str());
                if (selectedNodes[i].node && selectedNodes[i].node->getLayer())
                    ImGui::Text("Layer ID:         %s", selectedNodes[i].node->getLayer()->id.c_str());
                drawImGuiWidgetTexture(handle, bitmap->width(), bitmap->height(), zoom, selectedNodes.size());
            }
        }
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void RGIWindow::drawImageComparisonWidget(float &zoom) {
    data->prevImageComaprisonParams = data->imageComparisonParams;

    ImGui::Begin("Image comparison");

    ImGui::Combo("Comparison type [-Q][+E]", &data->imageComparisonParams.selectedComparisonType, rgiImageComparisonTypes, rgiImageComparisonTypesCount);

    const bool isSlider = data->imageComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::SLIDER);
    const bool isDiffWeightSlider = data->imageComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::DIFF);
    const bool isSideBySide = data->imageComparisonParams.selectedComparisonType == 3;

    if (isSlider) {
        ImGui::SliderFloat("Slider X position [-A][+D]", &data->imageComparisonParams.sliderXPos, 0.0f, 1.0f);
    }
    if (isDiffWeightSlider) {
        ImGui::SliderFloat("Diff weight [-A][+D]", &data->imageComparisonParams.diffWeight, 0.0f, 100.0f);
    }

    const RGIRenderedNodes selectedNodes = data->getSelectedNodes();
    if (selectedNodes.size() == 2) {
        const Internal::TexturesContext &context = data->texturesContext;
        const BitmapPtr &bitmapL = selectedNodes.front().bitmap;
        const BitmapPtr &bitmapR = selectedNodes.back().bitmap;

        if (isSideBySide && context.compareTextures[0] != nullptr && context.compareTextures[1] != nullptr) {
            ImGui::Columns(2);
            drawImGuiWidgetTexture(context.compareTextures[0]->getInternalGLHandle(), bitmapL->width(), bitmapL->height(), zoom, 2);

            ImGui::NextColumn();
            drawImGuiWidgetTexture(context.compareTextures[1]->getInternalGLHandle(), bitmapR->width(), bitmapR->height(), zoom, 2);

        } else if (data->imageComparisonParams.selectedComparisonType < 4 && data->texturesContext.compareTexture != nullptr) {
            drawImGuiWidgetTexture(context.compareTexture->getInternalGLHandle(), bitmapL->width(), bitmapL->height(), zoom);

        } else {
            ImGui::Text("---");
        }
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void RGIWindow::drawVersionsComparisonWidget(float &zoom) {
    data->prevImageVersionComaprisonParams = data->imageVersionComparisonParams;

    ImGui::Begin("Versions comparison");

    ImGui::Combo("Comparison type [-Q][+E]", &data->imageVersionComparisonParams.selectedComparisonType, rgiImageComparisonTypes, rgiImageComparisonTypesCount);

    const bool isSlider = data->imageVersionComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::SLIDER);
    const bool isDiffWeightSlider = data->imageVersionComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::DIFF);
    const bool isSideBySide = data->imageVersionComparisonParams.selectedComparisonType == 3;

    if (isSlider) {
        ImGui::SliderFloat("Slider X position [-A][+D]", &data->imageVersionComparisonParams.sliderXPos, 0.0f, 1.0f);
    }
    if (isDiffWeightSlider) {
        ImGui::SliderFloat("Diff weight [-A][+D]", &data->imageVersionComparisonParams.diffWeight, 0.0f, 100.0f);
    }

    const RGIRenderedNodes versionCompareSelectedNodes = data->getVersionCompareSelectedNodes();
    if (!data->loadedOctopus.renderGraphAltVersion.empty() && versionCompareSelectedNodes.size() == 2) {
        const Internal::TexturesContext &context = data->texturesContext;
        const BitmapPtr &bitmapL = versionCompareSelectedNodes.front().bitmap;

        if (isSideBySide && context.versionsComparisonTextures[0] != nullptr && context.versionsComparisonTextures[1] != nullptr) {
            ImGui::Columns(2);
            drawImGuiWidgetTexture(context.versionsComparisonTextures[0]->getInternalGLHandle(), bitmapL->width(), bitmapL->height(), zoom, 2);

            ImGui::NextColumn();
            drawImGuiWidgetTexture(context.versionsComparisonTextures[1]->getInternalGLHandle(), bitmapL->width(), bitmapL->height(), zoom, 2);

        } else if (data->imageVersionComparisonParams.selectedComparisonType < 4 && data->texturesContext.versionsComparisonTexture != nullptr) {
            drawImGuiWidgetTexture(context.versionsComparisonTexture->getInternalGLHandle(), bitmapL->width(), bitmapL->height(), zoom);

        } else {
            ImGui::Text("---");
        }
    } else {
        ImGui::Text("---");
    }

    ImGui::End();
}

void RGIWindow::handleKeyboardEvents() {
    const float sliderKeySpeed = 0.01f;
    const float diffWeightSliderKeySpeed = 0.05f;
    const float zoomKeySpeed = 0.03f;

    const bool isSlider = data->imageComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::SLIDER);
    const bool isDiffWeightSlider = data->imageComparisonParams.selectedComparisonType == static_cast<int>(ImageComparisonShader::Type::DIFF);

    if (isSlider) {
        if (ImGui::IsKeyDown(ImGuiKey_D)) {
            data->imageComparisonParams.moveSlider(sliderKeySpeed);
            data->imageVersionComparisonParams.moveSlider(sliderKeySpeed);
        } else if (ImGui::IsKeyDown(ImGuiKey_A)) {
            data->imageComparisonParams.moveSlider(-sliderKeySpeed);
            data->imageVersionComparisonParams.moveSlider(-sliderKeySpeed);
        }
    } else if (isDiffWeightSlider) {
        if (ImGui::IsKeyDown(ImGuiKey_D)) {
            data->imageComparisonParams.moveDiffWeight(diffWeightSliderKeySpeed);
            data->imageVersionComparisonParams.moveDiffWeight(diffWeightSliderKeySpeed);
        } else if (ImGui::IsKeyDown(ImGuiKey_A)) {
            data->imageComparisonParams.moveDiffWeight(-diffWeightSliderKeySpeed);
            data->imageVersionComparisonParams.moveDiffWeight(-diffWeightSliderKeySpeed);
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        data->zoomContext += zoomKeySpeed;
    } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
        data->zoomContext -= zoomKeySpeed;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        data->imageComparisonParams.selectedComparisonType = (data->imageComparisonParams.selectedComparisonType + 1) % rgiImageComparisonTypesCount;
        data->imageVersionComparisonParams.selectedComparisonType = (data->imageVersionComparisonParams.selectedComparisonType + 1) % rgiImageComparisonTypesCount;
    } else if (ImGui::IsKeyPressed(ImGuiKey_Q)) {
        data->imageComparisonParams.selectedComparisonType = (data->imageComparisonParams.selectedComparisonType - 1 + rgiImageComparisonTypesCount) % rgiImageComparisonTypesCount;
        data->imageVersionComparisonParams.selectedComparisonType = (data->imageVersionComparisonParams.selectedComparisonType - 1 + rgiImageComparisonTypesCount) % rgiImageComparisonTypesCount;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_1) || ImGui::IsKeyPressed(ImGuiKey_2) || ImGui::IsKeyPressed(ImGuiKey_3) || ImGui::IsKeyPressed(ImGuiKey_4) ||
        ImGui::IsKeyPressed(ImGuiKey_5) || ImGui::IsKeyPressed(ImGuiKey_6) || ImGui::IsKeyPressed(ImGuiKey_7) || ImGui::IsKeyPressed(ImGuiKey_8) || ImGui::IsKeyPressed(ImGuiKey_9)) {
        std::string searchString = std::string(data->nodeGraphContext.nodesSelectionStr);

        int keyNumber = 0;
        if (ImGui::IsKeyPressed(ImGuiKey_0)) keyNumber = 0;
        else if (ImGui::IsKeyPressed(ImGuiKey_1)) keyNumber = 1;
        else if (ImGui::IsKeyPressed(ImGuiKey_2)) keyNumber = 2;
        else if (ImGui::IsKeyPressed(ImGuiKey_3)) keyNumber = 3;
        else if (ImGui::IsKeyPressed(ImGuiKey_4)) keyNumber = 4;
        else if (ImGui::IsKeyPressed(ImGuiKey_5)) keyNumber = 5;
        else if (ImGui::IsKeyPressed(ImGuiKey_6)) keyNumber = 6;
        else if (ImGui::IsKeyPressed(ImGuiKey_7)) keyNumber = 7;
        else if (ImGui::IsKeyPressed(ImGuiKey_8)) keyNumber = 8;
        else if (ImGui::IsKeyPressed(ImGuiKey_9)) keyNumber = 9;

        if (searchString.front() == '#') {
            searchString.append(std::to_string(keyNumber));
        } else {
            searchString = "#" + std::to_string(keyNumber);
        }

        data->nodeGraphContext.selectionMode = RGINodeGraphContext::SelectionMode::SEARCH_NODES;
        memset(&(data->nodeGraphContext.nodesSelectionStr[0]), 0, sizeof(data->nodeGraphContext.nodesSelectionStr));
        strncpy(data->nodeGraphContext.nodesSelectionStr, searchString.c_str(), sizeof(data->nodeGraphContext.nodesSelectionStr)-1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && data->nodeGraphContext.selectionMode == RGINodeGraphContext::SelectionMode::SEARCH_NODES) {
        memset(&(data->nodeGraphContext.nodesSelectionStr[0]), 0, sizeof(data->nodeGraphContext.nodesSelectionStr));
    }
}
