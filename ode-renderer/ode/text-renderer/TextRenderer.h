
#pragma once

#include "../image/PlacedImage.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "shaders/TransformShader.h"

namespace ode {

class TextRenderer {

public:
    TextRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard);
    PlacedImagePtr drawLayerText(Component &component, const LayerInstanceSpecifier &layer, const ScaledBounds &visibleBounds, double scale, double time);

private:
    TextureFrameBufferManager &tfbManager;
    Mesh &billboard;
    TransformShader transformShader;

    PlacedImagePtr transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation);

};

}
