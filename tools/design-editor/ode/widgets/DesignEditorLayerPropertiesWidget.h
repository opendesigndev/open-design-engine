
#pragma once

#include <ode/logic-api.h>

#include "../DesignEditorContext.h"
#include "../DesignEditorLoadedOctopus.h"

/// Draw DE Layer Properties widget
void drawLayerPropertiesWidget(DesignEditorContext::Api &apiContext,
                               DesignEditorLoadedOctopus &loadedOctopus,
                               DesignEditorContext::LayerSelection &layerSelectionContext,
                               DesignEditorContext::FileDialog &fileDialogContext);
