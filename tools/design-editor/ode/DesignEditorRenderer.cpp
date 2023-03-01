
#include "DesignEditorRenderer.h"

namespace {
void clearColorBuffer(int selectedDisplayMode) {
    switch (selectedDisplayMode) {
        case 0: glClearColor(0.0f, 0.0f, 0.0f, 0.0f); break;
        case 1: glClearColor(0.0f, 0.0f, 0.0f, 1.0f); break;
        case 2: glClearColor(1.0f, 1.0f, 1.0f, 1.0f); break;
        case 3: glClearColor(0.0f, 0.0f, 0.0f, 0.0f); break;
    }
    glClear(GL_COLOR_BUFFER_BIT);
}

inline ScaledBounds convertToScaledBounds(const PixelBounds &pb) {
    return ScaledBounds(pb.a.x, pb.a.y, pb.b.x, pb.b.y);
}
}

DesignEditorRenderer::DesignEditorRenderer()
   : sharedVertexShader("diagnostics-shared-vs") {
   const bool sharedVsInitialized = sharedVertexShader.initialize();
   if (!sharedVsInitialized) {
       fprintf(stderr, "Failed to initialize vertex shader\n");
       return;
   }

   blitShader.initialize(sharedVertexShader);
   const float billboardVertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
   int attributeSize = 2;
   billboard.initialize(billboardVertices, &attributeSize, 1, GL_TRIANGLES, 6);
}

TexturePtr DesignEditorRenderer::blendImageToTexture(const BitmapPtr &bitmap, const ScaledBounds &placement, int selectedDisplayMode) {
   const bool ignoreAlpha = selectedDisplayMode == 3;

   ImagePtr image = Image::fromBitmap(bitmap, Image::NORMAL);
   TexturePtr texture = image->asTexture();

   PixelBounds bounds = outerPixelBounds(placement);
   ScaledBounds sBounds = convertToScaledBounds(bounds);

   bind();

   // Bind framebuffer
   TextureFrameBufferPtr outTex = tfbm.acquire(bounds);
   outTex->bind();

   // Clear the background with the specified color
   glViewport(0, 0, bounds.dimensions().x, bounds.dimensions().y);
   clearColorBuffer(selectedDisplayMode);

   blitShader.bind(sBounds, sBounds, 0, ignoreAlpha);

   // Draw the image texture to framebuffer blended on top of the background
   texture->bind(0);
   billboard.draw();
   outTex->unbind();
   ODE_CHECK_GL_ERROR();

   unbind();

   return outTex;
}

void DesignEditorRenderer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVbo);
}

void DesignEditorRenderer::unbind() {
    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, prevVbo);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glDisable(GL_BLEND);
}
