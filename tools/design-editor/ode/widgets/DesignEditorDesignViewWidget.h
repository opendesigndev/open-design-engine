
#pragma once

#include <ode/logic-api.h>
#include <ode/renderer-api.h>

#include "../DesignEditorRenderer.h"
#include "../DesignEditorContext.h"
#include "../DesignEditorUIState.h"

/// Draw DE Design View widget
void drawDesignViewWidget(const ODE_ComponentHandle &component,
                          const ODE_Bitmap &bitmap,
                          DesignEditorRenderer &renderer,
                          DesignEditorUIState::Mode uiMode,
                          DesignEditorUIState::Textures &texturesContext,
                          DesignEditorUIState::Canvas &canvasContext,
                          const DesignEditorUIState::LayerSelection &layerSelection,
                          const ODE_StringRef &topLayerId,
                          int selectedDisplayMode);
