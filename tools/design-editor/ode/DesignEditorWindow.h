
#pragma once

#include <memory>
#include <ode-essentials.h>

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
    bool readManifestFile(const ode::FilePath &manifestPath);
    /// Read the specified octopus file - single component
    bool readOctopusFile(const ode::FilePath &octopusPath);

    void setImageDirectory(const ode::FilePath &imageDirectory_);
    void setFontDirectory(const ode::FilePath &fontDirectory);
    void setIgnoreValidation(bool ignoreValidation);

private:
    void drawControlsWidget();

    void handleKeyboardEvents();

    ode::FilePath imageDirectory;
    ode::FilePath fontDirectory;
    bool ignoreValidation = false;

    struct Internal;
    std::unique_ptr<Internal> data;
};
