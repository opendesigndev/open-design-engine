
#pragma once

#include <ode-logic.h>

using namespace ode;

/// Representation of a Render Graph Inspector window and its widgets.
class RGIWindow {
public:
    /// Get singleton instance
    static RGIWindow& getInstance();

private:
    /// Initalize the render graph inspector window with dimensions
    explicit RGIWindow(int width_, int height_);
    ~RGIWindow();

public:
    RGIWindow(RGIWindow const&) = delete;
    void operator=(RGIWindow const&) = delete;

    /// Display the window - initialize and run its main loop
    int display();
    /// Read the specified octopus file
    bool readOctopusFile(const FilePath &octopusPath);

    void setImageDirectory(const FilePath &imageDirectory_);
    void setFontDirectory(const FilePath &fontDirectory);
    void setIgnoreValidation(bool ignoreValidation);

private:
    void drawControlsWidget();
    void drawRenderGraphWidget();
    void drawRenderGraphInfoWidget();
    void drawResultImageWidget(float &zoom);
    void drawSelectedImagesWidget(float &zoom);
    void drawImageComparisonWidget(float &zoom);
    void drawVersionsComparisonWidget(float &zoom);

    void handleKeyboardEvents();

    FilePath imageDirectory;
    FilePath fontDirectory;
    bool ignoreValidation = false;

    struct Internal;
    std::unique_ptr<Internal> data;
};
