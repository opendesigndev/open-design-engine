
#pragma once

#include <ode/logic-api.h>

#include "../DesignEditorContext.h"

/// Draw DE Layer Properties widget
void drawLayerPropertiesWidget(ODE_LayerList &layerList,
                               DesignEditorContext::Api &apiContext,
                               DesignEditorContext::LayerSelection &layerSelectionContext);
