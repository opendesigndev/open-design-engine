
#include "FontAtlas.h"

// TODO
#include "../../../open-design-text-renderer/src/text-renderer/Context.h"
#include "../../../open-design-text-renderer/src/text-renderer/Face.h"
#include "../../../open-design-text-renderer/src/fonts/FontManager.h"
#include "../../../open-design-text-renderer/src/fonts/FaceTable.h"

#include <ode/text-renderer/text-renderer-instance.h>

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

FontAtlas::TextureStorage::TextureStorage() = default;

FontAtlas::TextureStorage::TextureStorage(int width, int height) : texture(FilterMode::LINEAR) {
    texture.initialize(PixelFormat::R, Vector2i(width, height));
}

FontAtlas::TextureStorage::TextureStorage(const TextureStorage &orig, int width, int height) {
    texture.initialize(PixelFormat::R, Vector2i(width, height));
    // TODO copy texture from orig.texture
}

FontAtlas::TextureStorage::TextureStorage(const TextureStorage &orig, int width, int height, const msdf_atlas::Remap *remapping, int count) {
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

FontAtlas::FontAtlas() : atlas(msdf_atlas::ImmediateAtlasGenerator<float, 1, &msdf_atlas::psdfGenerator, TextureStorage>(FONT_ATLAS_INITIAL_SIZE, FONT_ATLAS_INITIAL_SIZE)), baseScale(1./MSDF_ATLAS_DEFAULT_EM_SIZE) { }

bool FontAtlas::initialize(const odtr::FontSpecifier &fontSpecifier) {
    if (fontHandle)
        return true;
    const odtr::FaceTable::Item *faceItem = TEXT_RENDERER_CONTEXT->getFontManager().facesTable().getFaceItem(fontSpecifier.faceId);
    if (!(faceItem && faceItem->face))
        return false;
    fontHandle = FontHandleHolder(faceItem->face->getFtFace());
    msdfgen::FontMetrics fontMetrics;
    if (msdfgen::getFontMetrics(fontMetrics, fontHandle))
        baseScale = 1./fontMetrics.emSize;
    return true;
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
    glyphQuads.insert(it, std::make_pair(glyphIndex, output));
    return FONT_ATLAS_PIXEL_RANGE/FONT_ATLAS_SCALE;
}

}
