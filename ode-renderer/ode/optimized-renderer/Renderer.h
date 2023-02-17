
#pragma once

#include <map>
#include <octopus/octopus.h>
#include <ode-essentials.h>
#include <ode-rasterizer.h>
#include <ode-logic.h>
#include <ode-graphics.h>
#include "../image/Image.h"
#include "../image/ImageBase.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "EffectRenderer.h"
#include "compositing-shaders/compositing-shaders.h"
#include "fill-shaders/fill-shaders.h"
#include "TransformShader.h"

namespace ode {

/// Facilitates the render process of the render expression tree
class Renderer {

public:
    explicit Renderer(GraphicsContext &gc);

    PlacedImagePtr blend(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode);
    PlacedImagePtr blendIgnoreAlpha(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode);
    PlacedImagePtr mask(const PlacedImagePtr &image, const PlacedImagePtr &mask, ChannelMatrix channelMatrix);
    PlacedImagePtr mixMask(const PlacedImagePtr &a, const PlacedImagePtr &b, const PlacedImagePtr &mask, ChannelMatrix channelMatrix);
    PlacedImagePtr mix(const PlacedImagePtr &a, const PlacedImagePtr &b, double ratio);
    PlacedImagePtr multiplyAlpha(const PlacedImagePtr &image, double multiplier);

    PlacedImagePtr drawLayerBody(Component &component, const LayerInstanceSpecifier &layer, double scale, double time);
    PlacedImagePtr drawLayerStroke(Component &component, const LayerInstanceSpecifier &layer, int index, double scale, double time);
    PlacedImagePtr drawLayerFill(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, double scale, double time);
    PlacedImagePtr drawLayerStrokeFill(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, double scale, double time);
    PlacedImagePtr drawLayerText(Component &component, const LayerInstanceSpecifier &layer, double scale, double time);
    PlacedImagePtr drawLayerEffect(Component &component, const LayerInstanceSpecifier &layer, int index, ImageBase &imageBase, const PlacedImagePtr &basis, double scale, double time);
    PlacedImagePtr applyFilter(const octopus::Filter &filter, const PlacedImagePtr &basis);

    PlacedImagePtr reframe(const PlacedImagePtr &image, const PixelBounds &bounds);

    void screenDraw(const PixelBounds &viewport, const PlacedImagePtr &image, const Color &bgColor);

    // Free up some memory
    void cleanUp();

private:
    GraphicsContext &gc;
    Rasterizer rasterizer;
    TextureFrameBufferManager tfbManager;
    EffectRenderer effectRenderer;

    PlacedImagePtr resolveAlphaChannel(const PlacedImagePtr &image);
    PlacedImagePtr transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation);
    PlacedImagePtr blend(const PlacedImagePtr &dst, const PlacedImagePtr &src, octopus::BlendMode blendMode, bool ignoreSrcAlpha);
    PlacedImagePtr drawLayerVector(Component &component, const LayerInstanceSpecifier &layer, int strokeIndex, double scale, double time);
    PlacedImagePtr drawFill(Component &component, const LayerInstanceSpecifier &layer, ImageBase &imageBase, const octopus::Fill &fill, double scale, double time);

    Mesh billboard;

    Texture2D transparentTexture;
    Texture2D whiteTexture;

    CompositingShader::SharedResource compositingShaderRes;
    FillShader::SharedResource fillShaderRes;
    SolidColorShader solidColorShader;
    BlitShader blitShader;
    MixShader mixShader;
    MixMaskShader mixMaskShader;
    AlphaMultShader alphaMultShader;
    TransformShader transformShader;
    std::map<octopus::BlendMode, BlendShader> blendShaders;
    std::map<octopus::Gradient::Type, GradientFillShader> gradientFillShaders;
    std::map<int, ImageFillShader> imageFillShaders;

};

}
