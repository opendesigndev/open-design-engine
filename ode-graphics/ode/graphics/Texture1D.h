
#pragma once

#include <ode/graphics/pixel-format.h>
#include <ode/graphics/FilterMode.h>
#include "gl.h"

namespace ode {

/// Represents an OpenGL 1-dimensional texture
class Texture1D {

public:
    explicit Texture1D(FilterMode filter = FilterMode::LINEAR, bool wrap = false);
    Texture1D(const Texture1D &) = delete;
    ~Texture1D();
    Texture1D &operator=(const Texture1D &) = delete;
    bool initialize(const void *pixels, int width, PixelFormat format);
    void bind(int slot) const;

private:
    GLuint handle;
    int width;
    size_t memorySize;

};

}
