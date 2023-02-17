
#pragma once

#include <ode/geometry/Rectangle.h>
#include "gl.h"
#include "Texture2D.h"
#include "RenderBuffer.h"

namespace ode {

class GraphicsContext;

/// Represents an OpenGL framebuffer object
class FrameBuffer {
    friend class GraphicsContext;
    //DEBUG_INSTANCE_COUNTER(FrameBuffer)

public:
    /// Copies pixels from src to dst
    static void blit(FrameBuffer *dst, const FrameBuffer *src, const Rectangle<int> &area);
    /// Copies pixels from src to dst
    static void blit(Texture2D *dst, const FrameBuffer *src, const Rectangle<int> &area);
    /// Copies pixels from src to dst
    static void blit(FrameBuffer *dst, const FrameBuffer *src, const Rectangle<int> &dstArea, const Rectangle<int> &srcArea);
    /// Copies pixels from src to dst
    static void blit(Texture2D *dst, const FrameBuffer *src, const Rectangle<int> &dstArea, const Rectangle<int> &srcArea);
    /// Unbinds any active framebuffer object, causing the screen (window surface) to be the target framebuffer
    static void bindScreen();

    FrameBuffer();
    FrameBuffer(const FrameBuffer &) = delete;
    FrameBuffer(FrameBuffer &&orig);
    ~FrameBuffer();
    FrameBuffer &operator=(const FrameBuffer &) = delete;
    FrameBuffer &operator=(FrameBuffer &&orig);
    /// Checks if the framebuffer is properly initialized
    explicit operator bool() const;
    /// Links the framebuffer with a texture as storage
    void setOutput(Texture2D *texture);
    /// Links the framebuffer with a render buffer as storage
    void setOutput(RenderBuffer *renderBuffer);
    /// Unlinks the framebuffer from its texture
    void unsetOutput(Texture2D *texture);
    /// Unlinks the framebuffer from its render buffer
    void unsetOutput(RenderBuffer *renderBuffer);
    /// Binds the frame buffer as the rendering output
    void bind();
    /// Unbinds the frame buffer from being the rendering output
    void unbind();

private:
    GLuint handle;

};

}
