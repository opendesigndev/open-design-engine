
#pragma once

#include "shaders/CanvasShader.h"

using namespace ode;

struct DesignEditorRenderer {
public:
    DesignEditorRenderer();
    DesignEditorRenderer(const DesignEditorRenderer&) = delete;
    DesignEditorRenderer& operator=(const DesignEditorRenderer&) = delete;
    ~DesignEditorRenderer() = default;

    /// Blend image on a background by the specified selectedDisplayMode and set to texture
    TextureFrameBufferPtr blendImageToTexture(Bitmap &&bitmap,
                                   const ScaledBounds &placement,
                                   int selectedDisplayMode,
                                   const AnnotationRectangleOpt &selectionRectangle,
                                   const AnnotationRectangles &highlightRectangles);

private:
    void bind();
    void unbind();

    TextureFrameBufferManager tfbm;
    DesignEditorShader::SharedVertexShader sharedVertexShader;
    CanvasShader canvasShader;
    Mesh billboard;

    GLint prevFbo;
    GLint prevVbo;
};
