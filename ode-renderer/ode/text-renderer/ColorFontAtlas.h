
#pragma once

// PRIVATE - do not include in public headers

#ifdef ODE_REALTIME_TEXT_RENDERER

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <msdfgen-ext.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include <open-design-text-renderer/PlacedGlyph.h>
#include <ode-graphics.h>
#include "GlyphQuad.h"

namespace ode {

class TextRenderer;

class ColorFontAtlas {

public:
    ColorFontAtlas(TextRenderer *parentRenderer, const odtr::FontSpecifier &fontSpecifier);
    const Texture2D *getTexture() const;
    bool getGlyphQuad(GlyphQuad &output, unsigned glyphIndex);

private:
    FT_Face ftFace;

    class TextureStorage {
        Texture2D texture;
        TextRenderer *parentRenderer;
    public:
        TextureStorage(int width, int height, TextRenderer *parentRenderer);
        TextureStorage(TextureStorage &&orig, int width, int height);
        const Texture2D *get() const;
        void put(int x, int y, const msdfgen::BitmapConstRef<msdfgen::byte, 4> &subBitmap);
    };

    class Generator {
    public:
        Generator(int width, int height, TextRenderer *parentRenderer, FT_Face ftFace);
        void generate(const msdf_atlas::GlyphGeometry *glyphs, int count);
        void rearrange(int width, int height, const msdf_atlas::Remap *remapping, int count);
        void resize(int width, int height);
        const TextureStorage &atlasStorage() const;
        const std::vector<msdf_atlas::GlyphBox> &getLayout() const;

    private:
        TextureStorage storage;
        FT_Face ftFace;
        std::vector<msdf_atlas::GlyphBox> layout;
        std::vector<byte> glyphBuffer;
    };

    msdf_atlas::DynamicAtlas<Generator> atlas;
    std::map<unsigned, GlyphQuad> glyphQuads;
    double baseScale;

};

}

#endif
