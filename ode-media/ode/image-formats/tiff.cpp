
#include "tiff.h"

#ifdef ODE_MEDIA_TIFF_SUPPORT

#include <tiffio.h>

namespace ode {

class TiffFileGuard {
    TIFF *tiff;
public:
    inline explicit TiffFileGuard(TIFF *tiff) : tiff(tiff) { }
    TiffFileGuard(const TiffFileGuard &) = delete;
    inline ~TiffFileGuard() {
        TIFFCleanup(tiff);
    }
};

bool detectTiffFormat(const byte *data, size_t length) {
    return length >= 2 && (data[0] == 'I' || data[0] == 'M') && data[1] == data[0];
}

Bitmap loadTiff(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadTiff(file);
    return Bitmap();
}

Bitmap loadTiff(FILE *file) {
    ODE_ASSERT(file);
    if (TIFF *tiff = TIFFFdOpen(fileno(file), "", "r")) {
        TiffFileGuard tiffGuard(tiff);
        TIFF_UINT32_T width = 0, height = 0;
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
        if (width && height) {
            Bitmap bitmap(PixelFormat::RGBA, width, height);
            if (bitmap) {
                if (TIFFReadRGBAImageOriented(tiff, width, height, reinterpret_cast<uint32_t *>(bitmap.pixels()), ORIENTATION_TOPLEFT) == 1)
                    return bitmap;
            }
        }
    }
    return Bitmap();
}

bool saveTiff(const FilePath &path, SparseBitmapConstRef bitmap) {
    int channels = pixelChannels(bitmap.format);
    if (!(channels > 0 && channels <= 4 && bitmap.width() > 0 && bitmap.height() > 0 && bitmap.pixels))
        return false;
    if (FilePtr file = openFile(path, true)) {
        if (TIFF *tiff = TIFFFdOpen(fileno(file), "", "w")) {
            TiffFileGuard tiffGuard(tiff);
            if (!(
                TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, (TIFF_UINT32_T) bitmap.width()) &&
                TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, (TIFF_UINT32_T) bitmap.height()) &&
                TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, bitmap.format&PIXEL_FLOAT_BIT ? SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_UINT) &&
                TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, (TIFF_UINT16_T) channels) &&
                TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, (TIFF_UINT16_T) (bitmap.format&PIXEL_FLOAT_BIT ? 32 : 8)) &&
                TIFFSetField(tiff, TIFFTAG_ORIENTATION, (TIFF_UINT16_T) ORIENTATION_TOPLEFT) &&
                TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, (TIFF_UINT16_T) PLANARCONFIG_CONTIG) &&
                TIFFSetField(tiff, TIFFTAG_COMPRESSION, (TIFF_UINT16_T) COMPRESSION_NONE) &&
                TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, channels >= 3 ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK) &&
                TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, channels*bitmap.width()))
            ))
                return false;
            if (channels == 2 || channels == 4) {
                TIFF_UINT16_T extrasamples = pixelHasAlpha(bitmap.format) ? (bitmap.format&PIXEL_PREMULTIPLIED_BIT ? EXTRASAMPLE_ASSOCALPHA : EXTRASAMPLE_UNASSALPHA) : EXTRASAMPLE_UNSPECIFIED;
                if (!TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, (TIFF_UINT16_T) 1, &extrasamples))
                    return false;
            }
            for (int y = 0; y < bitmap.dimensions[1]; ++y) {
                if (!TIFFWriteScanline(tiff, const_cast<void *>(bitmap(0, y)), y))
                    return false;
            }
            return true;
        }
    }
    return false;
}

}

#endif
