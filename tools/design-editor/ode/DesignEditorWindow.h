
#pragma once

#include <memory>
#include <ode-essentials.h>

#include "DesignEditorRenderer.h"
#include "DesignEditorContext.h"
#include "DesignEditorUIState.h"

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

    /// Create a single-component empty design
    bool createEmptyDesign();
    /// Read the specified octopus file - single component
    bool readOctopusFile(const ode::FilePath &octopusPath);

    void setImageDirectory(const ode::FilePath &imageDirectory_);
    void setFontDirectory(const ode::FilePath &fontDirectory_);

private:
    void handleKeyboardEvents();

    int loadMissingFonts(const FilePath &fontDir);
    int createEmptyDesign(const FilePath &fontDir);
    int reloadOctopus(const FilePath &octopusPath, const FilePath &fontDir);

    ode::FilePath imageDirectory;
    ode::FilePath fontDirectory;

    std::unique_ptr<DesignEditorRenderer> renderer;

    DesignEditorContext context;
    DesignEditorUIState ui;
};
