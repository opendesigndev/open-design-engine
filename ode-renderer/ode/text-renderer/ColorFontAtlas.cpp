
#ifdef ODE_REALTIME_TEXT_RENDERER

// Make sure FreeType is included before first occurence of text-renderer-api.h
#include "ColorFontAtlas.h"

#include <open-design-text-renderer/text-renderer-api.h>
#include <ode/text-renderer/text-renderer-instance.h>
#include "TextRenderer.h"

#define FONT_ATLAS_INITIAL_SIZE 256

namespace ode {

static void unWhitespaceShape(msdfgen::Shape &shape, double, unsigned long long) {
    shape.contours.emplace_back();
}

static bool loadGlyphMetrics(msdf_atlas::GlyphGeometry &glyph, Rectangle<double> &planeBounds, FT_Face ftFace, unsigned glyphIndex) {
    if (FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_COLOR|FT_LOAD_NO_HINTING|FT_LOAD_IGNORE_TRANSFORM))
        return false;
    msdf_atlas::Rectangle rect = { };
    rect.w = ftFace->glyph->bitmap.width+1;
    rect.h = ftFace->glyph->bitmap.rows+1;

    // Make sure that atlas doesn't think it's a whitespace
    if (rect.w > 1 && rect.h > 1) {
        glyph.edgeColoring(&unWhitespaceShape, 0, 0);
        glyph.setBoxRect(rect);
    }

    double bitmapInvScale = 64./ftFace->size->metrics.height; // 64 because of 26.6 fixed point format
    planeBounds.a.x = bitmapInvScale*(ftFace->glyph->bitmap_left-.5);
    planeBounds.a.y = bitmapInvScale*(-ftFace->glyph->bitmap_top-.5);
    planeBounds.b.x = planeBounds.a.x+bitmapInvScale*rect.w;
    planeBounds.b.y = planeBounds.a.y+bitmapInvScale*rect.h;

    return true;
}

ColorFontAtlas::TextureStorage::TextureStorage(int width, int height, TextRenderer *parentRenderer) : texture(FilterMode::LINEAR, true), parentRenderer(parentRenderer) {
    // Must be blanked because of padding
    Bitmap blankBitmap(PixelFormat::RGBA, width, height);
    blankBitmap.clear();
    texture.initialize(blankBitmap);
}

ColorFontAtlas::TextureStorage::TextureStorage(TextureStorage &&orig, int width, int height) : texture(FilterMode::LINEAR, true), parentRenderer(orig.parentRenderer) {
    if (orig.texture.dimensions().x >= width && orig.texture.dimensions().y >= height) {
        texture = (Texture2D &&) orig.texture;
        return;
    }
    Bitmap blankBitmap(PixelFormat::RGBA, width, height);
    blankBitmap.clear();
    texture.initialize(blankBitmap);
    ODE_ASSERT(parentRenderer);
    if (parentRenderer)
        parentRenderer->blitTexture(texture, orig.texture);
}

const Texture2D *ColorFontAtlas::TextureStorage::get() const {
    return &texture;
}

void ColorFontAtlas::TextureStorage::put(int x, int y, const msdfgen::BitmapConstRef<msdfgen::byte, 4> &subBitmap) {
    texture.put(Vector2i(x, y), BitmapConstRef(PixelFormat::RGBA, subBitmap.pixels, subBitmap.width, subBitmap.height));
}

ColorFontAtlas::Generator::Generator(int width, int height, TextRenderer *parentRenderer, FT_Face ftFace) : storage(width, height, parentRenderer), ftFace(ftFace) { }

void ColorFontAtlas::Generator::generate(const msdf_atlas::GlyphGeometry *glyphs, int count) {
    int maxBoxArea = 0;
    for (int i = 0; i < count; ++i) {
        msdf_atlas::GlyphBox box = glyphs[i];
        maxBoxArea = std::max(maxBoxArea, box.rect.w*box.rect.h);
        layout.push_back((msdf_atlas::GlyphBox &&) box);
    }
    int threadBufferSize = 4*maxBoxArea;
    if (threadBufferSize > (int) glyphBuffer.size())
        glyphBuffer.resize(threadBufferSize);

    for (int i = 0; i < count; ++i) {
        const msdf_atlas::GlyphGeometry &glyph = glyphs[i];
        int l, b, w, h;
        glyph.getBoxRect(l, b, w, h);
        if (w > 1 && h > 1) {
            --w, --h;
            memcpy(glyphBuffer.data(), ftFace->glyph->bitmap.buffer, 4*w*h);
            BitmapRef bitmapRef(PixelFormat::RGBA, glyphBuffer.data(), w, h);
            bitmapPremultiply(bitmapRef);
            storage.put(l, b, msdfgen::BitmapConstRef<byte, 4>(glyphBuffer.data(), w, h));
        }
    }
}

void ColorFontAtlas::Generator::rearrange(int width, int height, const msdf_atlas::Remap *remapping, int count) {
    ODE_ASSERT(!"Not implemented - make sure to never call DynamicAtlas::add with allowRearrange = true");
}

void ColorFontAtlas::Generator::resize(int width, int height) {
    TextureStorage newStorage((TextureStorage &&) storage, width, height);
    storage = (TextureStorage &&) newStorage;
}

const ColorFontAtlas::TextureStorage &ColorFontAtlas::Generator::atlasStorage() const {
    return storage;
}

const std::vector<msdf_atlas::GlyphBox> &ColorFontAtlas::Generator::getLayout() const {
    return layout;
}

ColorFontAtlas::ColorFontAtlas(TextRenderer *parentRenderer, const odtr::FontSpecifier &fontSpecifier) :
    ftFace(odtr::getFreetypeFace(TEXT_RENDERER_CONTEXT, fontSpecifier.faceId)),
    atlas(FONT_ATLAS_INITIAL_SIZE, parentRenderer, ftFace),
    baseScale(1./MSDF_ATLAS_DEFAULT_EM_SIZE)
{ }

const Texture2D *ColorFontAtlas::getTexture() const {
    return atlas.atlasGenerator().atlasStorage().get();
}

bool ColorFontAtlas::getGlyphQuad(GlyphQuad &output, unsigned glyphIndex) {
    std::map<unsigned, GlyphQuad>::const_iterator it = glyphQuads.find(glyphIndex);
    if (it != glyphQuads.end()) {
        output = it->second;
        return true;
    }
    msdf_atlas::GlyphGeometry glyph;
    if (!loadGlyphMetrics(glyph, output.planeBounds, ftFace, glyphIndex))
        return false;
    atlas.add(&glyph, 1, false);
    glyph.getQuadAtlasBounds(output.atlasBounds.a.x, output.atlasBounds.a.y, output.atlasBounds.b.x, output.atlasBounds.b.y);
    output.atlasBounds.a.x -= 1;
    output.atlasBounds.a.y -= 1;
    glyphQuads.insert(it, std::make_pair(glyphIndex, output));
    return true;
}

}

#endif
