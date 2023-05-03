
#include "DesignEditorWindow.h"

#include <filesystem>

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
#include "widgets/DesignEditorControlsWidget.h"
#include "widgets/DesignEditorToolbarWidget.h"
#include "widgets/DesignEditorLayerListWidget.h"
#include "widgets/DesignEditorDesignViewWidget.h"
#include "widgets/DesignEditorLayerPropertiesWidget.h"
#include "widgets/DesignEditorUIHelpers.h"

namespace {

#define CHECK(x) do { if ((x)) return -1; } while (false)

struct {
    const Vector2f size { 1920, 1080 };
    const Color color { 1.0, 1.0, 1.0, 1.0 };
    const std::string rootId = "COMPONENT_ROOT";
    const std::string backgroundId = "BACKGROUND";
} DEFAULT_NEW_COMPONENT;

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

ODE_Vector2 toImageSpace(const ImVec2 &posInScreenSpace, const DesignEditorUIState::Canvas &canvas, const DesignEditorComponent &component) {
    // Image space position from top layer bounds
    const ODE_StringRef &topLayerId = component.layerList.entries[0].id;

    const ImVec2 inCanvasSpace = ImVec2 {
        (posInScreenSpace.x - canvas.bbMin.x) / canvas.bbSize.x,
        (posInScreenSpace.y - canvas.bbMin.y) / canvas.bbSize.y,
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

}


/*static*/ DesignEditorWindow& DesignEditorWindow::getInstance() {
    static DesignEditorWindow instance;
    return instance;
}

DesignEditorWindow::DesignEditorWindow() {
    context.initialize();
    renderer = std::make_unique<DesignEditorRenderer>();
}

DesignEditorWindow::~DesignEditorWindow() {
    ui.textures.designImageTexture.reset();
    renderer.reset();
    context.destroy();
}

int DesignEditorWindow::display() {
    // Get the GLFW window as initialized in ODE GraphicsContext
    GLFWwindow *window = reinterpret_cast<GraphicsContext *>(context.rc.ptr)->getNativeHandle<GLFWwindow *>();

    // Set file drop callback
    glfwSetDropCallback(window, fileDropCallback);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup fonts to be displayed in the UI
    setupUIFonts();

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
        drawControlsWidget(context.design, ui);

        if (!context.design.empty()) {
            DesignEditorComponent &component = context.design.components.back();

            // Mouse click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ui.canvas.isMouseOver) {
                const ODE_Vector2 mousePosImageSpace = toImageSpace(ImGui::GetMousePos(), ui.canvas, component);

                // If a single group or mask group is selected, insert into it, otherwise to the top layer
                const ODE_StringRef &topLayerID = component.layerList.entries[0].id;
                ODE_StringRef insertionLayerId = topLayerID;
                const std::vector<ODE_StringRef> &selectedLayerIds = ui.layerSelection.layerIDs;

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
                if (ui.mode == DesignEditorUIState::Mode::ADD_RECTANGLE ||
                    ui.mode == DesignEditorUIState::Mode::ADD_ELLIPSE ||
                    ui.mode == DesignEditorUIState::Mode::ADD_TEXT) {
                    const octopus::Octopus octopus = [mode = ui.mode, &layerList = component.layerList]()->octopus::Octopus {
                        if (mode == DesignEditorUIState::Mode::ADD_RECTANGLE) {
                            ode::octopus_builder::ShapeLayer rectangleShape(0, 0, DEFAULT_NEW_SHAPE_SIZE.x, DEFAULT_NEW_SHAPE_SIZE.y);
                            rectangleShape.setColor(DEFAULT_NEW_SHAPE_COLOR);
                            const std::optional<std::string> idOpt = findAvailableLayerId("RECTANGLE", layerList);
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
                        loadMissingFonts(fontDirectory);

                        const ODE_Transformation translation { 1, 0, 0, 1, std::round(mousePosImageSpace.x), std::round(mousePosImageSpace.y) };
                        ode_component_listLayers(component.component, &component.layerList);
                        const ODE_StringRef insertedLayerId = lastChildLayerId(component.layerList, insertionLayerId);
                        if (insertedLayerId.data!=nullptr && insertedLayerId.length>=0) {
                            ode_component_transformLayer(component.component, insertedLayerId, ODE_TRANSFORMATION_BASIS_LAYER, translation);
                        }

                        ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
                    }

                } else if (ui.mode == DesignEditorUIState::Mode::SELECT || ui.mode == DesignEditorUIState::Mode::MOVE) {
                    ODE_String selectedLayerId;
                    ode_component_identifyLayer(component.component, &selectedLayerId, mousePosImageSpace, 2.0f);
                    if (isImGuiMultiselectKeyModifierPressed()) {
                        ui.layerSelection.add(selectedLayerId.data);
                    } else {
                        ui.layerSelection.select(selectedLayerId.data);
                    }
                }
            }

            // Mouse rectangle selection
            if (ui.mode == DesignEditorUIState::Mode::SELECT &&
                ui.canvas.mouseClickPos.has_value() &&
                ui.canvas.mouseDragPos.has_value()) {
                const ODE_Vector2 rectStartInImageSpace = toImageSpace(*ui.canvas.mouseClickPos, ui.canvas, component);
                const ODE_Vector2 rectEndInImageSpace = toImageSpace(*ui.canvas.mouseDragPos, ui.canvas, component);
                const ODE_Rectangle selectionRectangle {
                    ODE_Vector2 { std::min(rectStartInImageSpace.x, rectEndInImageSpace.x), std::min(rectStartInImageSpace.y, rectEndInImageSpace.y) },
                    ODE_Vector2 { std::max(rectStartInImageSpace.x, rectEndInImageSpace.x), std::max(rectStartInImageSpace.y, rectEndInImageSpace.y) } };

                ui.layerSelection.clear();

                for (int i = 0; i < component.layerList.n; ++i) {
                    const ODE_LayerList::Entry &layer = component.layerList.entries[i];

                    ODE_LayerMetrics layerMetrics;
                    ode_component_getLayerMetrics(component.component, layer.id, &layerMetrics);

                    if (isRectangleIntersection(selectionRectangle, layerMetrics.transformedGraphicalBounds)) {
                        ui.layerSelection.add(layer.id.data);
                    }
                }
            }
            // Mouse move
            if (ui.mode == DesignEditorUIState::Mode::MOVE &&
                !ui.layerSelection.empty() &&
                ui.canvas.mouseDragPos.has_value() &&
                ui.canvas.prevMouseDragPos.has_value() &&
                (ui.canvas.mouseDragPos->x != ui.canvas.prevMouseDragPos->x || ui.canvas.mouseDragPos->y != ui.canvas.prevMouseDragPos->y)) {
                const ODE_Vector2 prevPosInImageSpace = toImageSpace(*ui.canvas.prevMouseDragPos, ui.canvas, component);
                const ODE_Vector2 posInImageSpace = toImageSpace(*ui.canvas.mouseDragPos, ui.canvas, component);

                const ODE_Scalar trX = posInImageSpace.x - prevPosInImageSpace.x;
                const ODE_Scalar trY = posInImageSpace.y - prevPosInImageSpace.y;

                const ODE_Vector2 translationInImageSpace = ImGui::IsKeyDown(ImGuiKey_LeftShift)
                    ? ODE_Vector2 { std::round(trX), std::round(trY) }
                    : ODE_Vector2 { trX, trY };;

                for (const ODE_StringRef &layerId : ui.layerSelection.layerIDs) {
                    const ODE_Transformation newTransformation { 1,0,0,1,translationInImageSpace.x,translationInImageSpace.y };
                    if (ode_component_transformLayer(component.component, layerId, ODE_TRANSFORMATION_BASIS_PARENT_COMPONENT, newTransformation) == ODE_RESULT_OK) {
                        ode_pr1_drawComponent(context.rc, component.component, context.design.imageBase, &component.bitmap, context.frameView);
                    }
                }
            }

            if (ui.canvas.isMouseOver) {
                const ODE_Vector2 mousePosImageSpace = toImageSpace(ImGui::GetMousePos(), ui.canvas, component);
                ImGui::SetTooltip("[%.2f, %.2f]", mousePosImageSpace.x, mousePosImageSpace.y);
            }

            if (ui.widgets.showToolbar) {
                drawToolbarWidget(context, component, ui.layerSelection, ui.mode);
            }
            if (ui.widgets.showLayerList) {
                drawLayerListWidget(component.layerList, ui.layerSelection);
            }
            if (ui.widgets.showDesignView) {
                drawDesignViewWidget(component.component, component.bitmap, *renderer, ui.mode, ui.textures, ui.canvas, ui.layerSelection, component.layerList.entries[0].id, ui.imageVisualizationParams.selectedDisplayMode);
            }
            if (ui.widgets.showLayerProperties) {
                drawLayerPropertiesWidget(context, component, ui.layerSelection, ui.fileDialog);
            }
        }

        if (ui.widgets.showImGuiDebugger) {
            ImGui::ShowMetricsWindow();
        }

        // Rendering
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        const ImVec4 &clearColor = ui.imGuiWindow.clearColor;
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

bool DesignEditorWindow::createEmptyDesign() {
    return createEmptyDesign(fontDirectory) == 0;
}

bool DesignEditorWindow::readOctopusFile(const FilePath &octopusPath) {
    return reloadOctopus(octopusPath, fontDirectory) == 0;
}

void DesignEditorWindow::setImageDirectory(const FilePath &imageDirectory_) {
    imageDirectory = imageDirectory_;
}

void DesignEditorWindow::setFontDirectory(const FilePath &fontDirectory_) {
    fontDirectory = fontDirectory_;
}

void DesignEditorWindow::handleKeyboardEvents() {
    const float zoomKeySpeed = 0.03f;
    const float minZoom = 1.0f;
    const float maxZoom = 10.0f;

    if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) || ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        if (ImGui::IsKeyDown(ImGuiKey_1)) {
            ui.mode = DesignEditorUIState::Mode::SELECT;
        } else if (ImGui::IsKeyDown(ImGuiKey_2)) {
            ui.mode = DesignEditorUIState::Mode::MOVE;
        } else if (ImGui::IsKeyDown(ImGuiKey_3)) {
            ui.mode = DesignEditorUIState::Mode::ADD_RECTANGLE;
        } else if (ImGui::IsKeyDown(ImGuiKey_4)) {
            ui.mode = DesignEditorUIState::Mode::ADD_ELLIPSE;
        } else if (ImGui::IsKeyDown(ImGuiKey_5)) {
            ui.mode = DesignEditorUIState::Mode::ADD_TEXT;
        }

        if (ImGui::IsKeyDown(ImGuiKey_W)) {
            ui.canvas.zoom = std::clamp(ui.canvas.zoom + zoomKeySpeed, minZoom, maxZoom);
        } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
            ui.canvas.zoom = std::clamp(ui.canvas.zoom - zoomKeySpeed, minZoom, maxZoom);
        }
    }
}

int DesignEditorWindow::loadMissingFonts(const FilePath &fontDir) {
    ODE_StringList missingFonts;
    CHECK(ode_design_listMissingFonts(context.design.design, &missingFonts));

    for (int i = 0; i < missingFonts.n; ++i) {
        if (missingFonts.entries[i].length > 0) {
            const std::string pathStr = (std::string)fontDir+std::string("/")+ode_stringDeref(missingFonts.entries[i])+std::string(".ttf");
            const ODE_Result result = ode_design_loadFontFile(context.design.design, missingFonts.entries[i], ode_stringRef(pathStr), ODE_StringRef());
            if (result != ODE_RESULT_OK && result != ODE_RESULT_FONT_ERROR) {
                return result;
            }
        }
    }
    return 0;
}

int DesignEditorWindow::createEmptyDesign(const FilePath &fontDir) {
    ui.layerSelection.clear();

    // TODO: Allow multiple components
    if (context.design.design.ptr) {
        CHECK(ode_destroyDesign(context.design.design));
        CHECK(ode_createDesign(context.engine, &context.design.design));
    }
    context.design.imageDirectory = imageDirectory.empty() ? std::filesystem::temp_directory_path().string()+"/ode-design-editor-images" : imageDirectory;
    // TODO: set imageBase directory using the ODE API
    reinterpret_cast<ImageBase *>(context.design.imageBase.ptr)->setImageDirectory(context.design.imageDirectory.parent());

    context.design.components.clear();
    DesignEditorComponent &newComponent = context.design.components.emplace_back();

    ode::octopus_builder::ShapeLayer backgroundShape(0, 0, DEFAULT_NEW_COMPONENT.size.x, DEFAULT_NEW_COMPONENT.size.y);
    backgroundShape.setColor(DEFAULT_NEW_COMPONENT.color);
    backgroundShape.id = DEFAULT_NEW_COMPONENT.backgroundId;
    backgroundShape.name = DEFAULT_NEW_COMPONENT.backgroundId;
    ode::octopus_builder::MaskGroupLayer maskGroup(octopus::MaskBasis::BODY, backgroundShape);
    maskGroup.id = DEFAULT_NEW_COMPONENT.rootId;
    maskGroup.name = DEFAULT_NEW_COMPONENT.rootId;
    octopus::Octopus oc = ode::octopus_builder::buildOctopus("Mask Group", maskGroup);
    oc.dimensions = { DEFAULT_NEW_COMPONENT.size.x, DEFAULT_NEW_COMPONENT.size.y };

    octopus::Serializer::serialize(newComponent.octopusJson, oc);

    CHECK(ode_design_addComponentFromOctopusString(context.design.design, &newComponent.component, newComponent.metadata, ode_stringRef(newComponent.octopusJson), nullptr));
    CHECK(loadMissingFonts(fontDir));
    CHECK(ode_component_listLayers(newComponent.component, &newComponent.layerList));
    CHECK(ode_pr1_drawComponent(context.rc, newComponent.component, context.design.imageBase, &newComponent.bitmap, context.frameView));
    return 0;
}

int DesignEditorWindow::reloadOctopus(const FilePath &octopusPath, const FilePath &fontDir) {
    ui.layerSelection.clear();

    // TODO: Allow multiple components
    if (context.design.design.ptr) {
        CHECK(ode_destroyDesign(context.design.design));
        CHECK(ode_createDesign(context.engine, &context.design.design));
    }

    context.design.components.clear();
    DesignEditorComponent &newComponent = context.design.components.emplace_back();

    if (!readFile(octopusPath, newComponent.octopusJson)) {
        fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
        return false;
    }
    context.design.imageDirectory = imageDirectory.empty() ? (std::string)octopusPath.parent()+"/images" : imageDirectory;
    // TODO: set imageBase directory using the ODE API
    reinterpret_cast<ImageBase *>(context.design.imageBase.ptr)->setImageDirectory(context.design.imageDirectory.parent());

    CHECK(ode_design_addComponentFromOctopusString(context.design.design, &newComponent.component, newComponent.metadata, ode_stringRef(newComponent.octopusJson), nullptr));
    CHECK(loadMissingFonts(fontDir));
    CHECK(ode_component_listLayers(newComponent.component, &newComponent.layerList));
    CHECK(ode_pr1_drawComponent(context.rc, newComponent.component, context.design.imageBase, &newComponent.bitmap, context.frameView));

    if (ui.fileDialog.octopusFilePath.empty()) {
        ui.fileDialog.octopusFilePath = (std::string)octopusPath.parent();
        ui.fileDialog.octopusFileName = octopusPath.filename();
    }

    return 0;
}

void DesignEditorWindow::setupUIFonts() {
    ImGuiIO& io = ImGui::GetIO();

    if (std::filesystem::exists((std::string)fontDirectory + "Ayuthaya.ttf")) {
        io.Fonts->AddFontFromFileTTF(((std::string)fontDirectory + "Ayuthaya.ttf").c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesThai());
    }
    if (std::filesystem::exists((std::string)fontDirectory + "Arial.ttf")) {
        io.Fonts->AddFontFromFileTTF(((std::string)fontDirectory + "Arial.ttf").c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    }
}
