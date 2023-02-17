
#pragma once

#include <memory>
#include <ode/math/Vector2.h>

namespace ode {

struct GraphicsContextData;
class FrameBuffer;

/// An OpenGL graphics context (may also manage its own window depending on platform)
class GraphicsContext {

public:
    enum Offscreen { OFFSCREEN };

    /// Returns the side of the maximum size texture supported by the OpenGL implementation
    static int getMaxTextureSize();
    /// Returns the side of the maximum size render buffer supported by the OpenGL implementation
    static int getMaxRenderBufferSize();

    explicit GraphicsContext(const Vector2i &dimensions);
    GraphicsContext(const char *label, const Vector2i &dimensions);
    GraphicsContext(Offscreen);
    GraphicsContext(Offscreen, const Vector2i &dimensions);
    GraphicsContext(const GraphicsContext &) = delete;
    ~GraphicsContext();
    GraphicsContext &operator=(const GraphicsContext &) = delete;
    /// Checks if the context is properly initialized
    explicit operator bool() const;
    /// Returns the dimensions of the primary output (screen) framebuffer
    Vector2i dimensions() const;
    /// Swaps the front and back buffer of the primary output framebuffer
    void swapOutputFramebuffer();
    /// Binds the primary output framebuffer as the rendering target
    void bindOutputFramebuffer();
    /// Sets a secondary framebuffer to act as the primary output framebuffer
    void spoofOutputFramebuffer(FrameBuffer *fb);
    /// Returns platform-specific handle of the context or its window
    template <typename T>
    T getNativeHandle();

private:
    class Internal;

    static constexpr Vector2i defaultDimensions = Vector2i(256, 256);

    static int maxTextureSize;
    static int maxRenderbufferSize;

    std::unique_ptr<Internal> data;

    void initialize(const char *label, const Vector2i &dimensions, bool offscreen);

};

}
