
#include "png.h"

#ifdef ODE_MEDIA_PNG_SUPPORT

#include <png.h>

namespace ode {

class PngReadStructGuard {
    png_structp png;
    png_infop info;
public:
    inline PngReadStructGuard(png_structp png, png_infop info) : png(png), info(info) { }
    PngReadStructGuard(const PngReadStructGuard &) = delete;
    inline ~PngReadStructGuard() {
        png_destroy_read_struct(&png, &info, NULL);
    }
};

class PngWriteStructGuard {
    png_structp png;
    png_infop info;
public:
    inline PngWriteStructGuard(png_structp png, png_infop info) : png(png), info(info) { }
    PngWriteStructGuard(const PngWriteStructGuard &) = delete;
    inline ~PngWriteStructGuard() {
        png_destroy_write_struct(&png, &info);
    }
};


static void pngError(png_structp png, png_const_charp message) {
    //Log::instance.log(Log::CORE_UTILS, Log::ERROR, std::string("Libpng error: ")+message);
}

static void pngWarning(png_structp png, png_const_charp message) {
    //Log::instance.log(Log::CORE_UTILS, Log::WARNING, std::string("Libpng warning: ")+message);
}

bool detectPngFormat(const byte *data, size_t length) {
    return length >= 4 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G';
}

Bitmap loadPng(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadPng(file);
    return Bitmap();
}

Bitmap loadPng(FILE *file) {
    ODE_ASSERT(file);
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, pngError, pngWarning);
    if (!png)
        return Bitmap();
    png_infop info = png_create_info_struct(png);
    PngReadStructGuard pngGuard(png, info);
    if (!info)
        return Bitmap();
    // Error handling (archaic longjump model)
    if (setjmp(png_jmpbuf(png)))
        return Bitmap();

    png_init_io(png, file);

    // (Here would be png_set_sig_bytes(pngPtr, 8); if we had consumed the signature already)
    // Read header and gather information
    png_read_info(png, info);
    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);
    png_uint_32 colorType = png_get_color_type(png, info);
    png_uint_32 transparency = png_get_valid(png, info, PNG_INFO_tRNS);

    // Make it so that indexed colors and less than 8 bits per channel are expanded into proper RGB/grayscale
    png_set_expand(png);
    // Make it so that 16bit channels are converted to regular 8bit ones
    png_set_strip_16(png);
    // Convert between grayscale and RGB
    if (!(colorType&PNG_COLOR_MASK_COLOR))
        png_set_gray_to_rgb(png);
    PixelFormat format = PixelFormat::RGBA;
    // Convert transparency
    if (transparency)
        png_set_tRNS_to_alpha(png);
    else if (!(colorType&PNG_COLOR_MASK_ALPHA))
        format = PixelFormat::RGB;

    Bitmap bitmap(format, width, height);
    if (bitmap) {
        // Create pointers to individual rows of the bitmap
        std::vector<png_bytep> rows;
        rows.resize(height);
        png_bytep cur = (byte *) bitmap;
        size_t stride = pixelSize(format)*bitmap.width();
        for (png_bytep &rowPtr : rows) {
            rowPtr = cur;
            cur += stride;
        }
        // Read the pixels
        png_read_image(png, rows.data());
    }
    return bitmap;
}

Bitmap loadPng(const byte *data, size_t length) {
    ODE_ASSERT(data);
    ODE_ASSERT(length > 0);
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, pngError, pngWarning);
    if (!png)
        return Bitmap();
    png_infop info = png_create_info_struct(png);
    PngReadStructGuard pngGuard(png, info);
    if (!info)
        return Bitmap();
    // Error handling (archaic longjump model)
    if (setjmp(png_jmpbuf(png)))
        return Bitmap();

    png_set_read_fn(png, &data, [](png_structp png_ptr, png_bytep data, png_size_t length) {
        typedef struct {
            const png_byte* data;
            const png_size_t size;
        } DataHandle;

        typedef struct {
            const DataHandle data;
            png_size_t offset;
        } ReadDataHandle;

        ReadDataHandle* handle = (ReadDataHandle*)png_get_io_ptr(png_ptr);
        const png_byte* png_src = handle->data.data + handle->offset;

        memcpy(data, png_src, length);
        handle->offset += length;
    });

    // (Here would be png_set_sig_bytes(pngPtr, 8); if we had consumed the signature already)
    // Read header and gather information
    png_read_info(png, info);
    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);
    png_uint_32 colorType = png_get_color_type(png, info);
    png_uint_32 transparency = png_get_valid(png, info, PNG_INFO_tRNS);

    // Make it so that indexed colors and less than 8 bits per channel are expanded into proper RGB/grayscale
    png_set_expand(png);
    // Make it so that 16bit channels are converted to regular 8bit ones
    png_set_strip_16(png);
    // Convert between grayscale and RGB
    if (!(colorType&PNG_COLOR_MASK_COLOR))
        png_set_gray_to_rgb(png);
    PixelFormat format = PixelFormat::RGBA;
    // Convert transparency
    if (transparency)
        png_set_tRNS_to_alpha(png);
    else if (!(colorType&PNG_COLOR_MASK_ALPHA))
        format = PixelFormat::RGB;

    Bitmap bitmap(format, width, height);
    if (bitmap) {
        // Create pointers to individual rows of the bitmap
        std::vector<png_bytep> rows;
        rows.resize(height);
        png_bytep cur = (byte *) bitmap;
        size_t stride = pixelSize(format)*bitmap.width();
        for (png_bytep &rowPtr : rows) {
            rowPtr = cur;
            cur += stride;
        }
        // Read the pixels
        png_read_image(png, rows.data());
    }
    return bitmap;
}

bool savePng(const FilePath &path, SparseBitmapConstRef bitmap) {
    if (!(bitmap.width() > 0 && bitmap.height() > 0))
        return false;
    ODE_ASSERT(bitmap.format == PixelFormat::R || bitmap.format == PixelFormat::RGB || bitmap.format == PixelFormat::RGBA);
    if (!(bitmap.format == PixelFormat::R || bitmap.format == PixelFormat::RGB || bitmap.format == PixelFormat::RGBA))
        return false;
    FilePtr file = openFile(path, true);
    if (!file)
        return false;
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, pngError, pngWarning);
    if (!png)
        return false;
    png_infop info = png_create_info_struct(png);
    PngWriteStructGuard pngGuard(png, info);
    if (!info)
        return false;
    // Error handling (archaic longjump model)
    if (setjmp(png_jmpbuf(png)))
        return false;
    png_init_io(png, file);

    std::vector<png_bytep> rows;
    rows.resize(bitmap.height());
    png_bytep cur = const_cast<png_bytep>((const byte *) bitmap);
    size_t stride = pixelSize(bitmap.format)*bitmap.width();
    for (png_bytep &rowPtr : rows) {
        rowPtr = cur;
        cur += stride;
    }
    int colorType = bitmap.format == PixelFormat::RGBA ? PNG_COLOR_TYPE_RGB_ALPHA : bitmap.format == PixelFormat::RGB ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY;
    png_set_IHDR(png, info, bitmap.width(), bitmap.height(), 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png, info, rows.data());
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    return true;
}

}

#endif
