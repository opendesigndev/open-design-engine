
#include "GraphicsContext.h"

#ifdef __EMSCRIPTEN__

#include <emscripten/html5.h>
#include "gl.h"

namespace ode {

class GraphicsContext::Internal {
public:
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE handle;
    bool ready;
};

int GraphicsContext::maxTextureSize = 0;
int GraphicsContext::maxRenderbufferSize = 0;

int GraphicsContext::getMaxTextureSize() {
    return maxTextureSize;
}

int GraphicsContext::getMaxRenderBufferSize() {
    return maxRenderbufferSize;
}

GraphicsContext::GraphicsContext(const Vector2i &dimensions) {
    initialize(nullptr, dimensions, false);
}

GraphicsContext::GraphicsContext(const char *target, const Vector2i &dimensions) {
    initialize(target, dimensions, false);
}

GraphicsContext::GraphicsContext(Offscreen) {
    initialize(nullptr, defaultDimensions, true);
}

GraphicsContext::GraphicsContext(Offscreen, const Vector2i &dimensions) {
    initialize(nullptr, dimensions, true);
}

void GraphicsContext::initialize(const char *target, const Vector2i &dimensions, bool offscreen) {
    data.reset(new Internal { });

    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.antialias = false;
    attributes.depth = false;
    attributes.premultipliedAlpha = false; // TODO TRUE?

    data->handle = emscripten_webgl_create_context(target, &attributes);
    if (data->handle <= 0)
        return;
    emscripten_webgl_make_context_current(data->handle);

    glDisable(GL_BLEND);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (!maxTextureSize) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        #if defined(ODE_DEBUG) && defined(ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE)
            maxTextureSize = ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE;
        #endif
        //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "GL_MAX_TEXTURE_SIZE = %d", maxTextureSize);
    }
    if (!maxRenderbufferSize) {
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
        #if defined(ODE_DEBUG) && defined(ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE)
            maxRenderbufferSize = ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE;
        #endif
        //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "GL_MAX_RENDERBUFFER_SIZE = %d", maxRenderbufferSize);
    }
    ODE_CHECK_GL_ERROR();
    //LOG_ACTION(GRAPHICS_CONTEXT_CONSTRUCTION, nullptr, width*height, "");
    data->ready = true;
}

GraphicsContext::~GraphicsContext() {
    if (data->handle)
        emscripten_webgl_destroy_context(data->handle);
    //LOG_OWN_ACTION(GRAPHICS_CONTEXT_DESTRUCTION, 1, "");
}

GraphicsContext::operator bool() const {
    return data->ready;
}

Vector2i GraphicsContext::dimensions() const {
    Vector2i result;
    if (data->handle)
        emscripten_webgl_get_drawing_buffer_size(data->handle, &result.x, &result.y);
    return result;
}

void GraphicsContext::swapOutputFramebuffer() { }

void GraphicsContext::bindOutputFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsContext::spoofOutputFramebuffer(FrameBuffer *) { }

}

#endif // __EMSCRIPTEN__
