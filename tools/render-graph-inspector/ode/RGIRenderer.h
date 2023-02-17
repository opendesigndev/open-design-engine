
#pragma once

#include "RGIImageComparisonParams.h"
#include "shaders/SimpleBlitShader.h"
#include "shaders/ImageComparisonShader.h"

using namespace ode;

struct RGIRenderer {
public:
    RGIRenderer();
    RGIRenderer(const RGIRenderer&) = delete;
    RGIRenderer& operator=(const RGIRenderer&) = delete;
    ~RGIRenderer() = default;

    /// Blend image on a background by the specified selectedDisplayMode and set to texture
    TexturePtr blendImageToTexture(const BitmapPtr &bitmap, const ScaledBounds &placement, int selectedDisplayMode);
    /// Compare two images, blend the comprison on a background by the specified selectedDisplayMode and set to texture
    TexturePtr compareImagesToTexture(const BitmapPtr &bitmapL, const BitmapPtr &bitmapR, const ScaledBounds &placementL, const ScaledBounds &placementR, int selectedDisplayMode, const RGIImageComparisonParams &params);

private:
    void bind();
    void unbind();

    TextureFrameBufferManager tfbm;
    RGIShader::SharedVertexShader sharedVertexShader;
    SimpleBlitShader blitShader;
    ImageComparisonShader diffShader;
    ImageComparisonShader sliderShader;
    ImageComparisonShader fadeShader;
    Mesh billboard;

    GLint prevFbo;
    GLint prevVbo;
};
