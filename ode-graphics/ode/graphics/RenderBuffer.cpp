
#include "RenderBuffer.h"

#include <ode/utils.h>
#include "GraphicsContext.h"

namespace ode {

RenderBuffer::RenderBuffer(const Vector2i &dimensions) : handle(0), dims(dimensions) {
    ODE_ASSERT(dimensions.x > 0 && dimensions.y > 0);
    if (dimensions.x > GraphicsContext::getMaxRenderBufferSize() || dimensions.y > GraphicsContext::getMaxRenderBufferSize()) {
        handle = 0;
        dims = Vector2i();
        return;
    }
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, dimensions.x, dimensions.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    ODE_CHECK_GL_ERROR();
    //ODE_DEBUG_ONLY(MemoryWatch::instance().registerChange(4ll*width_*height_, true));
    //LOG_OWN_ACTION(RBO_CREATION, 4*width_*height_, "");
}

RenderBuffer::RenderBuffer(RenderBuffer&& orig) : handle(orig.handle), dims(orig.dims) {
    orig.handle = 0;
    orig.dims = Vector2i();
}

RenderBuffer::~RenderBuffer() {
    if (handle) {
        glDeleteRenderbuffers(1, &handle);
        ODE_CHECK_GL_ERROR();
        //DEBUG_ONLY(MemoryWatch::instance().registerChange(-4ll*width_*height_, true));
        //LOG_OWN_ACTION(RBO_DELETION, 4*width_*height_, "");
    }
}

RenderBuffer &RenderBuffer::operator=(RenderBuffer &&orig) {
    if (this != &orig) {
        if (handle) {
            glDeleteRenderbuffers(1, &handle);
            ODE_CHECK_GL_ERROR();
            //DEBUG_ONLY(MemoryWatch::instance().registerChange(-4ll*width_*height_, true));
            //LOG_OWN_ACTION(RBO_DELETION, 4*width_*height_, "");
        }
        handle = orig.handle;
        dims = orig.dims;
        orig.handle = 0;
        orig.dims = Vector2i();
    }
    return *this;
}

RenderBuffer::operator bool() const {
    return handle != 0;
}

Vector2i RenderBuffer::dimensions() const {
    return dims;
}

}
