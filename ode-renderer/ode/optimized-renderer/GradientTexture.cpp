
#include "GradientTexture.h"

#include <cmath>
#include <ode-essentials.h>

#define GRADIENT_TEXTURE_WIDTH 256

namespace ode {

class GradientSampler {

    const std::vector<octopus::Gradient::ColorStop> &colorStops;
    size_t lastPos;

public:
    inline explicit GradientSampler(const std::vector<octopus::Gradient::ColorStop> &colorStops) : colorStops(colorStops), lastPos(0) { }
    Color operator()(double x);

};

static double wrapNegative(double x, int stepDif = 0) {
    if (x == 0 && stepDif < 0)
        return 1;
    return x+double(x < 0);
}

Color GradientSampler::operator()(double x) {
    constexpr bool wrap = false;
    size_t pos = lastPos;
    size_t a = 0, b = 0;
    Color color;
    // No color stops
    if (pos >= colorStops.size())
        return color;

    if (x < colorStops[pos].position)
        pos = 0;
    while (pos+1 < colorStops.size() && x > colorStops[pos+1].position)
        ++pos;
    if (pos == 0 && x < colorStops[pos].position) {
        if (wrap)
            a = colorStops.size()-1, b = 0;
        else
            a = 0, b = 0;
    } else if (pos+1 >= colorStops.size()) {
        if (wrap)
            a = pos, b = 0;
        else
            a = pos, b = pos;
    } else
        a = pos, b = pos+1;
    double width = wrapNegative(colorStops[b].position-colorStops[a].position, int(b)-int(a));
    double t = 0;
    if (width > 0) {
        t = wrapNegative(x-colorStops[a].position)/width;
        switch (colorStops[a].interpolation) {
            case octopus::Gradient::Interpolation::LINEAR:
                break;
            case octopus::Gradient::Interpolation::POWER:
                if (colorStops[a].interpolationParameter.has_value() && colorStops[a].interpolationParameter.value() > 0)
                    t = pow(t, colorStops[a].interpolationParameter.value());
                break;
            case octopus::Gradient::Interpolation::REVERSE_POWER:
                if (colorStops[a].interpolationParameter.has_value() && colorStops[a].interpolationParameter.value() > 0)
                    t = 1-pow(1-t, colorStops[a].interpolationParameter.value());
                break;
        }
    }
    color.r = (1-t)*colorStops[a].color.r + t*colorStops[b].color.r;
    color.g = (1-t)*colorStops[a].color.g + t*colorStops[b].color.g;
    color.b = (1-t)*colorStops[a].color.b + t*colorStops[b].color.b;
    color.a = (1-t)*colorStops[a].color.a + t*colorStops[b].color.a;
    return color;
}

GradientTexture::GradientTexture() : texture(FilterMode::LINEAR, false) {
    remap[0] = 1;
    remap[1] = 0;
}

bool GradientTexture::initialize(const octopus::Gradient &gradient) {
    return initialize(gradient.stops);
}

bool GradientTexture::initialize(const std::vector<octopus::Gradient::ColorStop> &colorStops) {
    // Gradient with no color data is invalid
    if (colorStops.empty())
        return false;
    byte pixels[4*GRADIENT_TEXTURE_WIDTH];
    // Special case - only one color stop - equivalent to solid color fill
    if (colorStops.size() == 1) {
        pixels[0] = channelFloatToByte(colorStops.front().color.r);
        pixels[1] = channelFloatToByte(colorStops.front().color.g);
        pixels[2] = channelFloatToByte(colorStops.front().color.b);
        pixels[3] = channelFloatToByte(colorStops.front().color.a);
        remap[0] = 1;
        remap[1] = 0;
        return texture.initialize(pixels, 1, 1, PixelFormat::RGBA);
    }
    // Special case - simple linear gradient with two colors
    if (colorStops.size() == 2 && colorStops.front().interpolation == octopus::Gradient::Interpolation::LINEAR && colorStops.front().position == 0 && colorStops.back().position == 1) {
        static_assert(GRADIENT_TEXTURE_WIDTH >= 2, "pixels array too small");
        pixels[0] = channelFloatToByte(colorStops.front().color.r);
        pixels[1] = channelFloatToByte(colorStops.front().color.g);
        pixels[2] = channelFloatToByte(colorStops.front().color.b);
        pixels[3] = channelFloatToByte(colorStops.front().color.a);
        pixels[4] = channelFloatToByte(colorStops.back().color.r);
        pixels[5] = channelFloatToByte(colorStops.back().color.g);
        pixels[6] = channelFloatToByte(colorStops.back().color.b);
        pixels[7] = channelFloatToByte(colorStops.back().color.a);
        /* Remap (texture coordinate transform 2x1 affine matrix) works as such:
         * For a 2-pixel wide texture, the middle of the two texels are at 0.25 and 0.75
         * We need to remap so that 0 maps to 0.25 and 1 maps to 0.75, or:
         *     remap[0] * 0 + remap[1] == 0.25
         *     remap[0] * 1 + remap[1] == 0.75
         */
        remap[0] = .5;
        remap[1] = .25;
        return texture.initialize(pixels, 2, 1, PixelFormat::RGBA);
    }
    // For the general case, sample the gradient and generate a one-dimensional texture
    GradientSampler gradientSampler(colorStops);
    for (int x = 0; x < GRADIENT_TEXTURE_WIDTH; ++x) {
        double t = 1./(GRADIENT_TEXTURE_WIDTH-1)*x;
        Color sample = gradientSampler(t);
        pixels[4*x+0] = channelFloatToByte(sample.r);
        pixels[4*x+1] = channelFloatToByte(sample.g);
        pixels[4*x+2] = channelFloatToByte(sample.b);
        pixels[4*x+3] = channelFloatToByte(sample.a);
    }
    /* In this case, the center of leftmost texel is at 0.5/GRADIENT_TEXTURE_WIDTH
     * and the center of the rightmost texel is at 1-0.5/GRADIENT_TEXTURE_WIDTH, so:
     *     remap[0] * 0 + remap[1] == 0.5/GRADIENT_TEXTURE_WIDTH
     *     remap[0] * 1 + remap[1] == 1-0.5/GRADIENT_TEXTURE_WIDTH
     */
    remap[0] = 1-1./GRADIENT_TEXTURE_WIDTH;
    remap[1] = .5/GRADIENT_TEXTURE_WIDTH;
    return texture.initialize(pixels, GRADIENT_TEXTURE_WIDTH, 1, PixelFormat::RGBA);
}

void GradientTexture::bind(int unit) const {
    texture.bind(unit);
}

Vector2f GradientTexture::transformation() const {
    return Vector2f(remap[0], remap[1]);
}

}
