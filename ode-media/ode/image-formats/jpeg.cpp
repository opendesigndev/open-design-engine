
#include "jpeg.h"

#ifdef ODE_MEDIA_JPEG_SUPPORT

#include <cstdint>
#include <csetjmp>
#include <jpeglib.h>
#include <jerror.h>

namespace ode {

enum class JpegOrientation {
    NONE = 0,
    IDENTITY = 1,
    HORIZONTAL_FLIP = 2,
    INVERSION = 3,
    VERTICAL_FLIP = 4,
    MAIN_DIAGONAL_FLIP = 5,
    CLOCKWISE_ROTATION = 6,
    OFF_DIAGONAL_FLIP = 7,
    COUNTER_CLOCKWISE_ROTATION = 8
};

struct JpegErrorHandlerData : jpeg_error_mgr {
    jmp_buf jumpDst;
};

class JpegDecompressGuard {
    j_decompress_ptr cinfo;
public:
    inline explicit JpegDecompressGuard(j_decompress_ptr cinfo) : cinfo(cinfo) { }
    JpegDecompressGuard(const JpegDecompressGuard &) = delete;
    inline ~JpegDecompressGuard() {
        jpeg_destroy_decompress(cinfo);
    }
};

static Bitmap loadJpeg(jpeg_decompress_struct &cinfo);
static Bitmap applyJpegOrientation(SparseBitmapConstRef bitmap, JpegOrientation orientation);

static uint16_t readUint16(const byte *data, bool bigEndian) {
    return uint16_t(bigEndian ?
        (unsigned(data[0])<<8|unsigned(data[1])) :
        (unsigned(data[0])|unsigned(data[1])<<8)
    );
}

static uint32_t readUint32(const byte *data, bool bigEndian) {
    return uint16_t(bigEndian ?
        (unsigned(data[0])<<24|unsigned(data[1])<<16|unsigned(data[2])<<8|unsigned(data[3])) :
        (unsigned(data[0])|unsigned(data[1])<<8|unsigned(data[2])<<16|unsigned(data[3])<<24)
    );
}

static void jpegErrorExit(j_common_ptr cinfo) {
#ifdef ODE_DEBUG
    char msgBuffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, msgBuffer);
    // TODO log msgBuffer as error
#endif
    longjmp(static_cast<JpegErrorHandlerData *>(cinfo->err)->jumpDst, 1);
}

static void jpegLogMessage(j_common_ptr cinfo) {
#ifdef ODE_DEBUG
    char msgBuffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, msgBuffer);
    // TODO log msgBuffer as warning (?)
#endif
}

static JpegOrientation jpegDetermineOrientation(j_decompress_ptr cinfo) {
    const byte tiffHeaderLE[] = { 0x49, 0x49, 0x2a, 0x00 };
    const byte tiffHeaderBE[] = { 0x4d, 0x4d, 0x00, 0x2a };
    jpeg_saved_marker_ptr exifMarker = nullptr;
    for (jpeg_saved_marker_ptr cur = cinfo->marker_list; cur; cur = cur->next) {
        if (cur->marker == JPEG_APP0+1 && !memcmp(cur->data, "Exif\0\0", 6)) {
            exifMarker = cur;
            break;
        }
    }
    if (!(exifMarker && exifMarker->data_length >= 32))
        return JpegOrientation::NONE;
    bool bigEndian = false;
    int pos = 0;
    for (; pos < 16; ++pos) {
        if (!memcmp(exifMarker->data+pos, tiffHeaderLE, sizeof(tiffHeaderLE))) {
            bigEndian = false;
            break;
        }
        if (!memcmp(exifMarker->data+pos, tiffHeaderBE, sizeof(tiffHeaderBE))) {
            bigEndian = true;
            break;
        }
    }
    if (pos < 16) {
        pos += readUint32(exifMarker->data+pos+4, bigEndian);
        if (pos+2 > int(exifMarker->data_length))
            return JpegOrientation::NONE;
        int tagCount = readUint16(exifMarker->data+pos, bigEndian);
        pos += 2;
        if (pos+12*tagCount > int(exifMarker->data_length))
            return JpegOrientation::NONE;
        for (int i = 0; i < tagCount; ++i) {
            if (
                readUint16(exifMarker->data+pos, bigEndian) == 0x0112u && // orientation tag
                readUint16(exifMarker->data+pos+2, bigEndian) == 3 && // type
                readUint32(exifMarker->data+pos+4, bigEndian) == 1 // value count
            ) {
                uint16_t orientation = readUint16(exifMarker->data+pos+8, bigEndian);
                if (orientation <= 8)
                    return JpegOrientation(orientation);
                break;
            }
            pos += 12;
        }
    }
    return JpegOrientation::NONE;
}

bool detectJpegFormat(const byte *data, size_t length) {
    return length >= 2 && data[0] == 0xff && data[1] == 0xd8;
}

#ifndef __EMSCRIPTEN__
Bitmap loadJpeg(const FilePath &path) {
    if (FilePtr file = openFile(path, false))
        return loadJpeg(file);
    return Bitmap();
}
#endif

#ifndef __EMSCRIPTEN__
Bitmap loadJpeg(FILE *file) {
    ODE_ASSERT(file);
    jpeg_decompress_struct cinfo;
    JpegErrorHandlerData jerr;
    jerr.error_exit = &jpegErrorExit;
    jerr.output_message = &jpegLogMessage;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    JpegDecompressGuard jpegGuard(&cinfo);
    if (setjmp(jerr.jumpDst))
        return Bitmap();
    jpeg_stdio_src(&cinfo, file);
    return loadJpeg(cinfo);
}
#endif

Bitmap loadJpeg(const byte *data, size_t length) {
    ODE_ASSERT(data);
    ODE_ASSERT(length > 0);
    jpeg_decompress_struct cinfo;
    JpegErrorHandlerData jerr;
    jerr.error_exit = &jpegErrorExit;
    jerr.output_message = &jpegLogMessage;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    JpegDecompressGuard jpegGuard(&cinfo);
    if (setjmp(jerr.jumpDst))
        return Bitmap();
    jpeg_mem_src(&cinfo, data, length);
    return loadJpeg(cinfo);
}

// Load jpeg helper

static Bitmap loadJpeg(jpeg_decompress_struct &cinfo) {
    JpegOrientation orientation = JpegOrientation::NONE;
    jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xffff);
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
        return Bitmap();
    orientation = jpegDetermineOrientation(&cinfo);
    if (!jpeg_start_decompress(&cinfo))
        return Bitmap();
    if (!(cinfo.output_components == 3 || cinfo.output_components == 1))
        return Bitmap();
    Bitmap bitmap(PixelFormat::RGB, cinfo.output_width, cinfo.output_height);
    if (bitmap) {
        size_t stride = pixelSize(bitmap.format())*bitmap.width();
        for (JSAMPROW row = reinterpret_cast<JSAMPROW>(bitmap.pixels()); cinfo.output_scanline < cinfo.output_height; row += stride) {
            static_assert(sizeof(*row) == 1, "Stride increment is in bytes");
            if (jpeg_read_scanlines(&cinfo, &row, 1) != 1)
                return Bitmap();
        }
        if (cinfo.output_components == 1) { // expand luminance -> RGB
            for (JSAMPROW row = reinterpret_cast<JSAMPROW>(bitmap.pixels()), end = row+stride*bitmap.height(); row < end; row += stride) {
                for (JSAMPLE *dst = row+stride, *src = row+bitmap.width(); dst -= 3, src-- > row;)
                    dst[0] = dst[1] = dst[2] = *src;
            }
        }
        jpeg_finish_decompress(&cinfo);
        if (orientation != JpegOrientation::NONE && orientation != JpegOrientation::IDENTITY)
            bitmap = applyJpegOrientation((Bitmap &&) bitmap, orientation);
    }
    return bitmap;
}

// Bitmap orientation change

static Bitmap applyJpegOrientation(SparseBitmapConstRef bitmap, JpegOrientation orientation) {
    Bitmap result;
    size_t pxSize = pixelSize(bitmap.format);
    switch (orientation) {
        case JpegOrientation::VERTICAL_FLIP:
            bitmap = bitmap.verticallyFlipped();
            // fallthrough
        case JpegOrientation::NONE:
        case JpegOrientation::IDENTITY:
            return Bitmap(bitmap);

        case JpegOrientation::INVERSION:
            bitmap = bitmap.verticallyFlipped();
            // fallthrough
        case JpegOrientation::HORIZONTAL_FLIP:
            if ((result = Bitmap(bitmap.format, bitmap.dimensions))) {
                for (int y = 0; y < bitmap.dimensions[1]; ++y) {
                    byte *dst = reinterpret_cast<byte *>(result(0, y));
                    const byte *src = reinterpret_cast<const byte *>(bitmap(bitmap.dimensions[0], y));
                    for (int i = 0; src -= pxSize, i < bitmap.dimensions[0]; dst += pxSize, ++i)
                        memcpy(dst, src, pxSize);
                }
            }
            break;

        case JpegOrientation::CLOCKWISE_ROTATION:
            bitmap = bitmap.verticallyFlipped();
            // fallthrough
        case JpegOrientation::MAIN_DIAGONAL_FLIP:
            if ((result = Bitmap(bitmap.format, bitmap.dimensions[1], bitmap.dimensions[0]))) {
                for (int sx = 0; sx < bitmap.dimensions[0]; ++sx) {
                    for (int sy = 0; sy < bitmap.dimensions[1]; ++sy)
                        memcpy(result(sy, sx), bitmap(sx, sy), pxSize);
                }
            }
            break;

        case JpegOrientation::COUNTER_CLOCKWISE_ROTATION:
            bitmap = bitmap.verticallyFlipped();
            // fallthrough
        case JpegOrientation::OFF_DIAGONAL_FLIP:
            if ((result = Bitmap(bitmap.format, bitmap.dimensions[1], bitmap.dimensions[0]))) {
                for (int sx = 0, dy = bitmap.dimensions[0]; --dy, sx < bitmap.dimensions[0]; ++sx) {
                    for (int sy = 0, dx = bitmap.dimensions[1]; --dx, sy < bitmap.dimensions[1]; ++sy)
                        memcpy(result(dx, dy), bitmap(sx, sy), pxSize);
                }
            }
            break;
    }
    return result;
}

}

#endif
