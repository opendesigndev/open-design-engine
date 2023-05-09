
#pragma once

#include <map>
#include "../image/PlacedImage.h"
#include "../frame-buffer-management/TextureFrameBufferManager.h"
#include "../optimized-renderer/compositing-shaders/BlitShader.h"
#include "shaders/SDFTextShader.h"
#include "shaders/ImageTextShader.h"
#include "shaders/TransformShader.h"

namespace odtr {
struct FontSpecifier;
}

namespace ode {

class FontAtlas;
class ColorFontAtlas;

class TextRenderer {

public:
    TextRenderer(GraphicsContext &gc, TextureFrameBufferManager &tfbManager, Mesh &billboard, BlitShader &blitShader);
    ~TextRenderer();
    TextRenderer(const TextRenderer &) = delete;
    FontAtlas &fontAtlas(const odtr::FontSpecifier &fontSpecifier);
    ColorFontAtlas &colorFontAtlas(const odtr::FontSpecifier &fontSpecifier);
    PlacedImagePtr drawLayerText(Component &component, const LayerInstanceSpecifier &layer, const ScaledBounds &visibleBounds, double scale, double time);

    void blitTexture(Texture2D &dst, const Texture2D &src);

private:
    TextureFrameBufferManager &tfbManager;
    Mesh &billboard;
    BlitShader &blitShader;
    SDFTextShader sdfShader;
    ImageTextShader imageShader;
    TransformShader transformShader;
#ifdef ODE_REALTIME_TEXT_RENDERER
    std::map<odtr::FontSpecifier, FontAtlas> fontAtlases;
    std::map<odtr::FontSpecifier, ColorFontAtlas> colorFontAtlases;
#endif

    PlacedImagePtr transformImage(const PlacedImagePtr &image, const Matrix3x3d &transformation);

};

}
