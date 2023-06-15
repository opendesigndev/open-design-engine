
#include "tiff.h"

#ifdef ODE_MEDIA_TIFF_SUPPORT

#include <sstream>

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

static Bitmap loadTiff(TIFF *tiff) {
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
    return Bitmap();
}

struct ReadDataHandle {
    const void* data;
    const size_t size;
    size_t offset;
};

static tmsize_t tiffReadProc(thandle_t fd, void *buf, tmsize_t size) {
    ReadDataHandle *dataHandle = reinterpret_cast<ReadDataHandle *>(fd);
    memcpy(buf, reinterpret_cast<const byte*>(dataHandle->data) + dataHandle->offset, size);
    dataHandle->offset += size;
    return size;
}

static tmsize_t tiffWriteProc(thandle_t, void *, tmsize_t) {
    return 0;
}

static uint64_t tiffSeekProc(thandle_t fd, uint64_t off, int whence) {
    ReadDataHandle *data = reinterpret_cast<ReadDataHandle *>(fd);
    switch (whence) {
        case SEEK_SET: {
            data->offset = off;
            break;
        }
        case SEEK_CUR: {
            data->offset += off;
            break;
        }
        case SEEK_END: {
            data->offset = data->size + off;
            break;
        }
    }
    return data->offset;
}

static uint64_t tiffSizeProc(thandle_t fd) {
    ReadDataHandle *data = reinterpret_cast<ReadDataHandle *>(fd);
    return data->size;
}

static int tiffCloseProc(thandle_t) {
    return 0;
}

static int tiffMapProc(thandle_t, tdata_t*, toff_t*) {
    return 0;
}

static void tiffUnmapProc(thandle_t, tdata_t, toff_t) {
    return;
}

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
        return loadTiff(tiff);
    }
    return Bitmap();
}

Bitmap loadTiff(const byte *data, size_t length) {
    ODE_ASSERT(data);
    ODE_ASSERT(length > 0);
    ReadDataHandle dataHandle { data, length, 0 };
    if (TIFF *tiff = TIFFClientOpen("memoryFile", "rm", reinterpret_cast<thandle_t>(&dataHandle), tiffReadProc,
                                    tiffWriteProc, tiffSeekProc, tiffCloseProc,
                                    tiffSizeProc, tiffMapProc, tiffUnmapProc)) {
        return loadTiff(tiff);
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
