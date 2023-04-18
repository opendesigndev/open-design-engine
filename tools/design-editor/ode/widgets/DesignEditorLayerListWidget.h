
#pragma once

#include <ode/logic-api.h>

#include "../DesignEditorUIState.h"

/// Draw DE Layer List widget
void drawLayerListWidget(const ODE_LayerList &layerList,
                         DesignEditorUIState::LayerSelection &layerSelection);
