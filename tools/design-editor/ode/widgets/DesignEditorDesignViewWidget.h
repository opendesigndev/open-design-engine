
#pragma once

#include "../DesignEditorRenderer.h"
#include "../DesignEditorContext.h"

/// Draw DE Design View widget
void drawDesignViewWidget(const ODE_Bitmap &bmp,
                          DesignEditorRenderer &renderer,
                          DesignEditorContext::Textures &texturesContext,
                          DesignEditorContext::Canvas &canvasContext);
