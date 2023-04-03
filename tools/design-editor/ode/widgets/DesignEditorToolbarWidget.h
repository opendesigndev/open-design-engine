
#pragma once

#include "../DesignEditorContext.h"

/// Draw DE Toolbar widget
void drawToolbarWidget(DesignEditorContext::Api &apiContext,
                       ODE_LayerList &layerList,
                       const DesignEditorContext::LayerSelection &layersSelectionContext,
                       DesignEditorMode &mode);
