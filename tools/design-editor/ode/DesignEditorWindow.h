
#pragma once

#include <memory>
#include <ode-essentials.h>

using namespace ode;

/// Representation of a Design Editor window and its widgets.
class DesignEditorWindow {
public:
    /// Get singleton instance
    static DesignEditorWindow& getInstance();

private:
    /// Initalize the render graph inspector window with dimensions
    DesignEditorWindow();
    ~DesignEditorWindow();

public:
    DesignEditorWindow(DesignEditorWindow const&) = delete;
    void operator=(DesignEditorWindow const&) = delete;

    /// Display the window - initialize and run its main loop
    int display();
    /// Read the specified octopus manifest file - multiple components
    bool readManifestFile(const FilePath &manifestPath);
    /// Read the specified octopus file - single component
    bool readOctopusFile(const FilePath &octopusPath);

    void setImageDirectory(const FilePath &imageDirectory_);
    void setFontDirectory(const FilePath &fontDirectory);
    void setIgnoreValidation(bool ignoreValidation);

private:
    void drawControlsWidget();
    void drawToolbarWidget();
    void drawLayerListWidget();
    void drawDesignViewWidget();
    void drawLayerPropertiesWidget();

    void handleKeyboardEvents();

    FilePath imageDirectory;
    FilePath fontDirectory;
    bool ignoreValidation = false;

    struct Internal;
    std::unique_ptr<Internal> data;
};
