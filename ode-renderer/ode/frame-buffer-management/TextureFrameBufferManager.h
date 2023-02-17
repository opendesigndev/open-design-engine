
#pragma once

#include <vector>
#include <map>
#include <ode-essentials.h>
#include <ode/core/bounds.h>
#include "TextureFrameBuffer.h"

namespace ode {

/// Manages a pool of reusable texture framebuffers
class TextureFrameBufferManager {

public:
    /// Provides a texture framebuffer with the specified or larger bounds
    TextureFrameBufferPtr acquire(PixelBounds &bounds);
    /// Provides a texture framebuffer with exactly the specified bounds
    TextureFrameBufferPtr acquireExact(const PixelBounds &bounds);
    /// Returns the texture framebuffer to the manager
    void relinquish(TextureFrameBuffer &&obj);

private:
    class DimsCmp {
    public:
        bool operator()(const Vector2i &a, const Vector2i &b) const;
    };

    std::map<Vector2i, std::vector<TextureFrameBuffer>, DimsCmp> stock;

};

}
