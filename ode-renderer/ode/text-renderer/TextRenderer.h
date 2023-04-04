
#pragma once

#include <map>
#include "../image/PlacedImage.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "shaders/SDFTextShader.h"
#include "shaders/TransformShader.h"

namespace odtr {
struct FontSpecifier;
}

namespace ode {

class FontAtlas;

class TextRenderer {

public:
    TextRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard);
    ~TextRenderer();
    TextRenderer(const TextRenderer &) = delete;
    FontAtlas &fontAtlas(const odtr::FontSpecifier &fontSpecifier);
    PlacedImagePtr drawLayerText(Component &component, const LayerInstanceSpecifier &layer, const ScaledBounds &visibleBounds, double scale, double time);

private:
    TextureFrameBufferManager &tfbManager;
    Mesh &billboard;
    SDFTextShader sdfShader;
    TransformShader transformShader;
    std::map<odtr::FontSpecifier, FontAtlas> fontAtlases;

    PlacedImagePtr transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation);

};

}
