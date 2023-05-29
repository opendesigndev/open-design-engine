
#pragma once

#include <ode/logic-api.h>
#include <ode/renderer-api.h>

#include "../DesignEditorRenderer.h"
#include "../DesignEditorContext.h"
#include "../DesignEditorUIState.h"

/// Draw DE Design View widget
void drawDesignViewWidget(const ODE_ComponentHandle &component,
                          const ODE_StringRef &componentId,
                          const ODE_Bitmap &bitmap,
                          DesignEditorRenderer &renderer,
                          ode::TextureFrameBufferPtr &texture,
                          DesignEditorUIState::Mode uiMode,
                          DesignEditorUIState::Canvas &canvas,
                          DesignEditorUIState::ComponentSelection &componentSelection,
                          const DesignEditorUIState::LayerSelection &layerSelection,
                          int selectedDisplayMode,
                          const ImVec2 &designViewPosition,
                          const ImVec2 &designViewSize);
