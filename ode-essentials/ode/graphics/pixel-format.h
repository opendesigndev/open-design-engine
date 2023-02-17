
#pragma once

#include <cstdlib>
#include "../utils.h"

namespace ode {

constexpr int PIXEL_CHANNEL_COUNT_MASK = 0x07;
constexpr int PIXEL_LUMINANCE_BIT = 0x08;
constexpr int PIXEL_ALPHA_ONLY_BIT = 0x10;
constexpr int PIXEL_PREMULTIPLIED_BIT = 0x20;
constexpr int PIXEL_LINEAR_BIT = 0x40;
constexpr int PIXEL_FLOAT_BIT = 0x80;

/// Bitmap / texture pixel format
enum class PixelFormat {
    EMPTY = 0,
    R = 1,
    RGB = 3,
    RGBA = 4,
    LUMINANCE = PIXEL_LUMINANCE_BIT|1,
    LUMINANCE_ALPHA = PIXEL_LUMINANCE_BIT|2,
    ALPHA = PIXEL_ALPHA_ONLY_BIT|1,
    PREMULTIPLIED_RGBA = PIXEL_PREMULTIPLIED_BIT|4,
    PREMULTIPLIED_LUMINANCE_ALPHA = PIXEL_PREMULTIPLIED_BIT|PIXEL_LUMINANCE_BIT|2,
    FLOAT_R = PIXEL_FLOAT_BIT|1,
    FLOAT_RGB = PIXEL_FLOAT_BIT|3,
    FLOAT_RGBA = PIXEL_FLOAT_BIT|4,
    FLOAT_LUMINANCE = PIXEL_FLOAT_BIT|PIXEL_LUMINANCE_BIT|1,
    FLOAT_LUMINANCE_ALPHA = PIXEL_FLOAT_BIT|PIXEL_LUMINANCE_BIT|2,
    FLOAT_ALPHA = PIXEL_FLOAT_BIT|PIXEL_ALPHA_ONLY_BIT|1,
    FLOAT_PREMULTIPLIED_RGBA = PIXEL_FLOAT_BIT|PIXEL_PREMULTIPLIED_BIT|4,
    FLOAT_PREMULTIPLIED_LUMINANCE_ALPHA = PIXEL_FLOAT_BIT|PIXEL_PREMULTIPLIED_BIT|PIXEL_LUMINANCE_BIT|2,
};

constexpr int operator&(PixelFormat format, int mask);

/// Returns the number of channels of a given pixel format
constexpr int pixelChannels(PixelFormat format);
/// Returns the size (in bytes) of a pixel in a given format
constexpr size_t pixelSize(PixelFormat format);
/// Returns true if the pixel format contains an alpha channel
constexpr bool pixelHasAlpha(PixelFormat format);
/// Returns true if the pixel format is alpha-premultiplied
constexpr bool isPixelPremultiplied(PixelFormat format);
/// Returns true if the pixel format is in linear color space
constexpr bool isPixelLinear(PixelFormat format);
/// Returns true if the pixel format has 32-bit floating-point channels
constexpr bool isPixelFloat(PixelFormat format);

/// Applies alpha-premultiplication to 8-bit color channel x
constexpr byte channelPremultiply(byte x, byte a);
/// Returns unpremultiplied color channel value of alpha-premultiplied 8-bit color channel x
constexpr byte channelUnpremultiply(byte x, byte a);

/// Converts 32-bit floating-point color channel value (0 to 1) to 8-bit unsigned integer value (0 to 255)
constexpr byte channelFloatToByte(float x);
/// Converts 64-bit floating-point color channel value (0 to 1) to 8-bit unsigned integer value (0 to 255)
constexpr byte channelFloatToByte(double x);
/// Converts 8-bit unsigned integer color channel value (0 to 255) to 32-bit floating-point value (0 to 1)
constexpr float channelByteToFloat(byte x);
/// Converts 8-bit unsigned integer color channel value (0 to 255) to 64-bit floating-point value (0 to 1)
constexpr double channelByteToDouble(byte x);

/// Converts 32-bit floating-point linear color channel value to its equivalent in sRGB color space
constexpr float sRGBfromLinear(float x);
/// Converts 64-bit floating-point linear color channel value to its equivalent in sRGB color space
constexpr double sRGBfromLinear(double x);
/// Converts 32-bit floating-point color channel value in sRGB color space to its linear equivalent
constexpr float sRGBtoLinear(float x);
/// Converts 64-bit floating-point color channel value in sRGB color space to its linear equivalent
constexpr double sRGBtoLinear(double x);
/// Converts 32-bit floating-point linear color channel value (0 to 1) to 8-bit unsigned integer value (0 to 255) in sRGB color space
constexpr byte sRGBbyteFromLinear(float x);
/// Converts 64-bit floating-point linear color channel value (0 to 1) to 8-bit unsigned integer value (0 to 255) in sRGB color space
constexpr byte sRGBbyteFromLinear(double x);
/// Converts 8-bit unsigned integer color channel value (0 to 255) in sRGB color space to 64-bit floating-point value (0 to 1) in linear color space
constexpr double sRGBbyteToLinear(byte x);

}

#include "pixel-format.hpp"
