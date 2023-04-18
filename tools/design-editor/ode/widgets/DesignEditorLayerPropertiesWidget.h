
#pragma once

#include <ode/logic-api.h>

#include "../DesignEditorContext.h"
#include "../DesignEditorComponent.h"
#include "../DesignEditorUIState.h"

/// Draw DE Layer Properties widget
void drawLayerPropertiesWidget(DesignEditorContext &context,
                               DesignEditorComponent &component,
                               DesignEditorUIState::LayerSelection &layerSelection,
                               DesignEditorUIState::FileDialog &fileDialogContext);
