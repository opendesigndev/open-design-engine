
#include "image-asset-generator.h"

#include <algorithm>

using namespace ode;

constexpr Vector2d complexMult(Vector2d a, Vector2d b) {
    return Vector2d(
        a.x*b.x - a.y*b.y,
        a.x*b.y + a.y*b.x
    );
}

constexpr double complexNorm(Vector2d v) {
    return v.x*v.x + v.y*v.y;
}

#ifndef ODE_DEBUG
#define MANDELBROT_ITERS 128
#else
#define MANDELBROT_ITERS 512
#endif

static double mandelbrot(Vector2d c) {
    Vector2d z(0.0);
    for (int i = 0; i < MANDELBROT_ITERS; i++) {
        z = complexMult(z, z)+c;
        double n = complexNorm(z);
        if (n > 256.0)
            return i-log2(log2(n));
    }
    return MANDELBROT_ITERS;
}

static void mandelbrotEx1Col(Color &color, double m, double q) {
    if (m >= MANDELBROT_ITERS) {
        color.a += 1;
        return;
    }
    double a = std::min(std::max((m-20)/16+q, 0.), 1.);
    m = log(m)+0.5*m;
    m *= -0.125;
    color.r += .5*pow(sin(1.1*m+.3250)+1, 1.5)*a;
    color.g += .5*pow(sin(1.0*m+.3750)+1, 1.5)*a;
    color.b += .5*pow(sin(1.0*m+.9375)+1, 1.5)*a;
    color.a += a;
}

static void generateMandelbrotEx1(BitmapRef bitmap, bool transparency) {
    ODE_ASSERT(pixelChannels(bitmap.format) == 4 && !isPixelFloat(bitmap.format) && pixelHasAlpha(bitmap.format));
    Vector2d aspect(
        std::max((double) bitmap.dimensions.x/bitmap.dimensions.y, 1.),
        std::max((double) bitmap.dimensions.y/bitmap.dimensions.x, 1.)
    );
    const double invzoom = 1/776.;
    Vector2d center(
        (invzoom-1)*1.0283-.5*invzoom,
        (1-invzoom)*.3617
    );
    Vector2d shift(
        .5*2.5*invzoom/bitmap.dimensions.x*aspect.x,
        .5*2.5*invzoom/bitmap.dimensions.y*aspect.y
    );
    Vector2d lc = center + 2.5*invzoom*Vector2d(-.5)*aspect;
    double q = transparency ? 0 : 999;
    byte *p = (byte *) bitmap;
    for (int y = 0; y < bitmap.dimensions.y; ++y) {
        Vector2d c = lc;
        for (int x = 0; x < bitmap.dimensions.x; ++x) {
            Color color(0, 0, 0, 0);
            mandelbrotEx1Col(color, mandelbrot(c), q);
            #ifndef ODE_DEBUG
                c.y += shift.y;
                mandelbrotEx1Col(color, mandelbrot(c), q);
                c.x += shift.x;
                mandelbrotEx1Col(color, mandelbrot(c), q);
                c.y -= shift.y;
                mandelbrotEx1Col(color, mandelbrot(c), q);
                c.x += shift.x;
                #define INV_SS .25
            #else
                c.x += 2*shift.x;
                #define INV_SS 1
            #endif
            *p++ = channelFloatToByte(INV_SS*color.r);
            *p++ = channelFloatToByte(INV_SS*color.g);
            *p++ = channelFloatToByte(INV_SS*color.b);
            *p++ = channelFloatToByte(INV_SS*color.a);
        }
        lc.y += 2*shift.y;
    }
    if (transparency && !isPixelPremultiplied(bitmap.format)) {
        p = (byte *) bitmap;
        for (int y = 0; y < bitmap.dimensions.y; ++y) {
            for (int x = 0; x < bitmap.dimensions.x; ++x) {
                p[0] = channelUnpremultiply(p[0], p[3]);
                p[1] = channelUnpremultiply(p[1], p[3]);
                p[2] = channelUnpremultiply(p[2], p[3]);
                p += 4;
            }
        }
    }
}

void generateImageAsset(BitmapRef bitmap, bool transparency) {
    return generateMandelbrotEx1(bitmap, transparency);
}
