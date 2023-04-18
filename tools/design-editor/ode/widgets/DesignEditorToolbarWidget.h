
#pragma once

#include "../DesignEditorContext.h"
#include "../DesignEditorComponent.h"
#include "../DesignEditorUIState.h"

/// Draw DE Toolbar widget
void drawToolbarWidget(DesignEditorContext &context,
                       DesignEditorComponent &component,
                       const DesignEditorUIState::LayerSelection &layersSelection,
                       DesignEditorUIState::Mode &mode);
