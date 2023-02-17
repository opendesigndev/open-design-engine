
#include "GraphicsContext.h"

#ifdef ODE_GRAPHICS_NO_CONTEXT

namespace ode {

class GraphicsContext::Internal { };

int GraphicsContext::maxTextureSize = 0;
int GraphicsContext::maxRenderbufferSize = 0;

int GraphicsContext::getMaxTextureSize() {
    return maxTextureSize;
}

int GraphicsContext::getMaxRenderBufferSize() {
    return maxRenderbufferSize;
}

GraphicsContext::GraphicsContext(const Vector2i &) { }

GraphicsContext::GraphicsContext(const char *, const Vector2i &) { }

GraphicsContext::GraphicsContext(Offscreen) { }

GraphicsContext::GraphicsContext(Offscreen, const Vector2i &) { }

GraphicsContext::~GraphicsContext() { }

GraphicsContext::operator bool() const {
    return false;
}

Vector2i GraphicsContext::dimensions() const {
    return Vector2i();
}

void GraphicsContext::swapOutputFramebuffer() { }

void GraphicsContext::bindOutputFramebuffer() { }

void GraphicsContext::spoofOutputFramebuffer(FrameBuffer *) { }

}

#endif // ODE_GRAPHICS_NO_CONTEXT
