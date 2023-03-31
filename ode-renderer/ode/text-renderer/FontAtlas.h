
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <msdfgen-ext.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include <ode-graphics.h>

namespace ode {

class FontAtlas {

public:
    FontAtlas();

private:
    class FontHandleHolder {
        msdfgen::FontHandle *handle;
    public:
        FontHandleHolder();
        explicit FontHandleHolder(FT_Face ftFace);
        FontHandleHolder(const FontHandleHolder &) = delete;
        FontHandleHolder(FontHandleHolder &&orig);
        ~FontHandleHolder();
        FontHandleHolder &operator=(const FontHandleHolder &) = delete;
        FontHandleHolder &operator=(FontHandleHolder &&orig);
        operator msdfgen::FontHandle *();
    } fontHandle;

    class TextureStorage {
        Texture2D texture;
    public:
        TextureStorage();
        TextureStorage(int width, int height);
        TextureStorage(const TextureStorage &orig, int width, int height);
        TextureStorage(const TextureStorage &orig, int width, int height, const msdf_atlas::Remap *remapping, int count);
        void put(int x, int y, const msdfgen::BitmapConstRef<float, 1> &subBitmap);
    };

    msdf_atlas::DynamicAtlas<msdf_atlas::ImmediateAtlasGenerator<float, 1, &msdf_atlas::psdfGenerator, TextureStorage> > atlas;

};

}
