
#pragma once

#include "../DesignEditorRenderer.h"
#include "../DesignEditorContext.h"

/// Draw DE Design View widget
void drawDesignViewWidget(const DesignEditorContext::Api &apiContext,
                          DesignEditorRenderer &renderer,
                          DesignEditorContext::Textures &texturesContext,
                          DesignEditorContext::Canvas &canvasContext,
                          const DesignEditorContext::LayerSelection &layerSelectionContext,
                          const ODE_StringRef &topLayerId);
