
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>

namespace ode {

class TextureFrameBufferManager;

/// A texture frame buffer managed by a TextureFrameBufferManager
class TextureFrameBuffer : public Texture2D {

public:
    explicit TextureFrameBuffer(TextureFrameBufferManager *parent);
    TextureFrameBuffer(const TextureFrameBuffer &) = delete;
    TextureFrameBuffer(TextureFrameBuffer &&orig) = default;
    TextureFrameBuffer(TextureFrameBuffer &&orig, TextureFrameBufferManager *parent);
    virtual ~TextureFrameBuffer();
    TextureFrameBuffer &operator=(const TextureFrameBuffer &) = delete;
    /// Initializes the framebuffer with the specified image dimensions
    bool initialize(const Vector2i &dimensions);
    /// Binds as texture to the specified texture unit
    void bind(int unit) const;
    /// Binds as the output rendering framebuffer
    void bind();
    /// Unbinds the framebuffer from being the rendering output
    void unbind();

private:
    TextureFrameBufferManager *parent;
    FrameBuffer frameBuffer;

};

typedef std::shared_ptr<TextureFrameBuffer> TextureFrameBufferPtr;

}
