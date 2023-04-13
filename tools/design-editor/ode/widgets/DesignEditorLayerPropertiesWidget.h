
#pragma once

#include <ode/logic-api.h>

#include "../DesignEditorContext.h"

/// Draw DE Layer Properties widget
void drawLayerPropertiesWidget(DesignEditorContext::Api &apiContext,
                               DesignEditorContext::LoadedOctopus &loadedOctopus,
                               DesignEditorContext::LayerSelection &layerSelectionContext,
                               DesignEditorContext::FileDialog &fileDialogContext);
