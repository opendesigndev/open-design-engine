
#include "RGIRenderer.h"

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

RGIRenderer::RGIRenderer() : sharedVertexShader("diagnostics-shared-vs") {
    const bool sharedVsInitialized = sharedVertexShader.initialize();
    if (!sharedVsInitialized) {
        fprintf(stderr, "Failed to initialize vertex shader\n");
        return;
    }

    blitShader.initialize(sharedVertexShader);
    diffShader.initialize(sharedVertexShader, ImageComparisonShader::Type::DIFF);
    sliderShader.initialize(sharedVertexShader, ImageComparisonShader::Type::SLIDER);
    fadeShader.initialize(sharedVertexShader, ImageComparisonShader::Type::FADE);
    const float billboardVertices[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
    int attributeSize = 2;
    billboard.initialize(billboardVertices, &attributeSize, 1, GL_TRIANGLES, 6);
}

TexturePtr RGIRenderer::blendImageToTexture(const BitmapPtr &bitmap, const ScaledBounds &placement, int selectedDisplayMode) {
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

TexturePtr RGIRenderer::compareImagesToTexture(const BitmapPtr &bitmapL, const BitmapPtr &bitmapR, const ScaledBounds &placementL, const ScaledBounds &placementR, int selectedDisplayMode, const RGIImageComparisonParams &params) {
    const bool isUnsupportedComparisonType = params.selectedComparisonType < static_cast<int>(ImageComparisonShader::Type::DIFF) || params.selectedComparisonType > static_cast<int>(ImageComparisonShader::Type::FADE);
    if (isUnsupportedComparisonType) {
        return nullptr;
    }

    const bool ignoreAlpha = selectedDisplayMode == 3;

    ImagePtr imageL = Image::fromBitmap(bitmapL, Image::NORMAL);
    TexturePtr textureL = imageL->asTexture();

    ImagePtr imageR = Image::fromBitmap(bitmapR, Image::NORMAL);
    TexturePtr textureR = imageR->asTexture();

    PixelBounds dstBounds = outerPixelBounds((placementL|placementR).canonical());
    if (!dstBounds)
        return nullptr;
    ScaledBounds dstSBounds = convertToScaledBounds(dstBounds);

    bind();

    // Bind framebuffer
    TextureFrameBufferPtr outTex = tfbm.acquire(dstBounds);
    outTex->bind();

    // Clear the background with the specified color
    glViewport(0, 0, dstBounds.dimensions().x, dstBounds.dimensions().y);
    clearColorBuffer(selectedDisplayMode);

    switch (params.selectedComparisonType) {
        case static_cast<int>(ImageComparisonShader::Type::DIFF):
            diffShader.bind(placementL, placementR, dstSBounds, 0, 1, params.sliderXPos, params.sliderStrokeWidth, params.diffWeight, ignoreAlpha);
            break;
        case static_cast<int>(ImageComparisonShader::Type::SLIDER):
            sliderShader.bind(placementL, placementR, dstSBounds, 0, 1, params.sliderXPos, params.sliderStrokeWidth, params.diffWeight,ignoreAlpha);
            break;
        case static_cast<int>(ImageComparisonShader::Type::FADE):
            fadeShader.bind(placementL, placementR, dstSBounds, 0, 1, params.sliderXPos, params.sliderStrokeWidth, params.diffWeight,ignoreAlpha);
            break;
    }

    // Draw the image texture to framebuffer blended on top of the background
    textureL->bind(0);
    textureR->bind(1);
    billboard.draw();
    outTex->unbind();
    ODE_CHECK_GL_ERROR();

    unbind();

    return outTex;
}

void RGIRenderer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevVbo);
}

void RGIRenderer::unbind() {
    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, prevVbo);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glDisable(GL_BLEND);
}
