
#ifdef ODE_REALTIME_TEXT_RENDERER

// Making sure FreeType is included before first occurence of text-renderer-api.h
#include <ft2build.h>
#include FT_FREETYPE_H

#include "FontAtlas.h"

#include <open-design-text-renderer/text-renderer-api.h>
#include <ode/text-renderer/text-renderer-instance.h>
#include "TextRenderer.h"

#define FONT_ATLAS_INITIAL_SIZE 256

#define FONT_ATLAS_SCALE 48.
#define FONT_ATLAS_PIXEL_RANGE 6.
#define FONT_ATLAS_MITER_LIMIT 1.

namespace ode {

FontAtlas::FontHandleHolder::FontHandleHolder() : handle(nullptr) { }

FontAtlas::FontHandleHolder::FontHandleHolder(FT_Face ftFace) : handle(msdfgen::adoptFreetypeFont(ftFace)) { }

FontAtlas::FontHandleHolder::FontHandleHolder(FontHandleHolder &&orig) : handle(orig.handle) {
    orig.handle = nullptr;
}

FontAtlas::FontHandleHolder::~FontHandleHolder() {
    if (handle)
        msdfgen::destroyFont(handle);
}

FontAtlas::FontHandleHolder &FontAtlas::FontHandleHolder::operator=(FontHandleHolder &&orig) {
    if (this != &orig) {
        if (handle)
            msdfgen::destroyFont(handle);
        handle = orig.handle;
        orig.handle = nullptr;
    }
    return *this;
}

FontAtlas::FontHandleHolder::operator msdfgen::FontHandle *() {
    return handle;
}

FontAtlas::TextureStorage::TextureStorage(int width, int height, TextRenderer *parentRenderer) : texture(FilterMode::LINEAR), parentRenderer(parentRenderer) {
    texture.initialize(PixelFormat::R, Vector2i(width, height));
}

FontAtlas::TextureStorage::TextureStorage(TextureStorage &&orig, int width, int height) : parentRenderer(orig.parentRenderer) {
    if (orig.texture.dimensions().x >= width && orig.texture.dimensions().y >= height) {
        texture = (Texture2D &&) orig.texture;
        return;
    }
    texture.initialize(PixelFormat::R, Vector2i(width, height));
    ODE_ASSERT(parentRenderer);
    if (parentRenderer)
        parentRenderer->blitTexture(texture, orig.texture);
}

FontAtlas::TextureStorage::TextureStorage(TextureStorage &&orig, int width, int height, const msdf_atlas::Remap *remapping, int count) : parentRenderer(orig.parentRenderer) {
    ODE_ASSERT(!"Not implemented - make sure to never call DynamicAtlas::add with allowRearrange = true");
}

const Texture2D *FontAtlas::TextureStorage::get() const {
    return &texture;
}

void FontAtlas::TextureStorage::put(int x, int y, const msdfgen::BitmapConstRef<float, 1> &subBitmap) {
    Bitmap byteBitmap(PixelFormat::R, subBitmap.width, subBitmap.height);
    const float *src = subBitmap.pixels;
    for (byte *dst = reinterpret_cast<byte *>(byteBitmap.pixels()), *dstEnd = dst+byteBitmap.size(); dst < dstEnd; ++dst, ++src)
        *dst = channelFloatToByte(*src);
    texture.put(Vector2i(x, y), byteBitmap);
}

FontAtlas::FontAtlas(TextRenderer *parentRenderer, const odtr::FontSpecifier &fontSpecifier) : atlas(FONT_ATLAS_INITIAL_SIZE, parentRenderer), baseScale(1./MSDF_ATLAS_DEFAULT_EM_SIZE) {
    FT_Face ftFace = odtr::getFreetypeFace(TEXT_RENDERER_CONTEXT, fontSpecifier.faceId);
    if (!ftFace)
        return;
    fontHandle = FontHandleHolder(ftFace);
    msdfgen::FontMetrics fontMetrics;
    if (msdfgen::getFontMetrics(fontMetrics, fontHandle))
        baseScale = 1./fontMetrics.emSize;
}

const Texture2D *FontAtlas::getTexture() const {
    return atlas.atlasGenerator().atlasStorage().get();
}

double FontAtlas::getGlyphQuad(GlyphQuad &output, unsigned glyphIndex) {
    std::map<unsigned, GlyphQuad>::const_iterator it = glyphQuads.find(glyphIndex);
    if (it != glyphQuads.end()) {
        output = it->second;
        return FONT_ATLAS_PIXEL_RANGE/FONT_ATLAS_SCALE;
    }
    msdf_atlas::GlyphGeometry glyph;
    if (!glyph.load(fontHandle, baseScale, msdfgen::GlyphIndex(glyphIndex)))
        return 0;
    glyph.wrapBox(FONT_ATLAS_SCALE, FONT_ATLAS_PIXEL_RANGE/FONT_ATLAS_SCALE, FONT_ATLAS_MITER_LIMIT);
    atlas.add(&glyph, 1, false);
    glyph.getQuadPlaneBounds(output.planeBounds.a.x, output.planeBounds.a.y, output.planeBounds.b.x, output.planeBounds.b.y);
    glyph.getQuadAtlasBounds(output.atlasBounds.a.x, output.atlasBounds.a.y, output.atlasBounds.b.x, output.atlasBounds.b.y);
    output.planeBounds.a.y = -output.planeBounds.a.y;
    output.planeBounds.b.y = -output.planeBounds.b.y;
    glyphQuads.insert(it, std::make_pair(glyphIndex, output));
    return FONT_ATLAS_PIXEL_RANGE/FONT_ATLAS_SCALE;
}

}

#endif
