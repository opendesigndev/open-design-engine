
#include "FontAtlas.h"

#define FONT_ATLAS_INITIAL_SIZE 256

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

void FontAtlas::TextureStorage::put(int x, int y, const msdfgen::BitmapConstRef<float, 1> &subBitmap) {
    Bitmap byteBitmap(PixelFormat::R, subBitmap.width, subBitmap.height);
    const float *src = subBitmap.pixels;
    for (byte *dst = reinterpret_cast<byte *>(byteBitmap.pixels()), *dstEnd = dst+byteBitmap.size(); dst < dstEnd; ++dst, ++src)
        *dst = channelFloatToByte(*src);
    texture.put(Vector2i(x, y), byteBitmap);
}

FontAtlas::FontAtlas() : atlas(msdf_atlas::ImmediateAtlasGenerator<float, 1, &msdf_atlas::psdfGenerator, TextureStorage>(FONT_ATLAS_INITIAL_SIZE, FONT_ATLAS_INITIAL_SIZE)) { }

}
