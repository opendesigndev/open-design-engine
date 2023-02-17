
#include "FrameBuffer.h"

#include <ode/utils.h>

#ifdef ODE_GRAPHICS_PEDANTIC
    #define PEDANTIC_ONLY(...) __VA_ARGS__
#else
    #define PEDANTIC_ONLY(...)
#endif

namespace ode {

#ifndef ODE_WEBGL_COMPATIBILITY
void FrameBuffer::blit(FrameBuffer *dst, const FrameBuffer *src, const Rectangle<int> &area) {
    blit(dst, src, area, area);
}
#endif

void FrameBuffer::blit(Texture2D *dst, const FrameBuffer *src, const Rectangle<int> &area) {
    blit(dst, src, area, area);
}

#ifndef ODE_WEBGL_COMPATIBILITY
void FrameBuffer::blit(FrameBuffer *dst, const FrameBuffer *src, const Rectangle<int> &dstArea, const Rectangle<int> &srcArea) {
    if (dstArea.a.x < dstArea.b.x && dstArea.a.y < dstArea.b.y && srcArea.a.x < srcArea.b.x && srcArea.a.y < srcArea.b.y) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->handle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->handle);
        glBlitFramebuffer(srcArea.a.x, srcArea.a.y, srcArea.b.x, srcArea.b.y, dstArea.a.x, dstArea.a.y, dstArea.b.x, dstArea.b.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        //LOG_ACTION(FBO_BLIT, source, srcArea.w*srcArea.h, "FBO -> FBO");
    }
}
#endif

void FrameBuffer::blit(Texture2D *dst, const FrameBuffer *src, const Rectangle<int> &dstArea, const Rectangle<int> &srcArea) {
    ODE_ASSERT(dstArea.dimensions() == srcArea.dimensions());
    if (dstArea.a.x < dstArea.b.x && dstArea.a.y < dstArea.b.y) {
        glBindFramebuffer(GL_FRAMEBUFFER, src->handle);
        glBindTexture(GL_TEXTURE_2D, dst->handle);
        if (dstArea.a == Vector2i() && dstArea.b == dst->dimensions())
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srcArea.a.x, srcArea.a.y, dstArea.b.x, dstArea.b.y, 0);
        else
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, dstArea.a.x, dstArea.a.y, srcArea.a.x, srcArea.a.y, dstArea.b.x-dstArea.a.x, dstArea.b.y-dstArea.a.y);
        PEDANTIC_ONLY(glBindTexture(GL_TEXTURE_2D, 0));
        PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        //LOG_ACTION(FBO_BLIT, source, srcArea.w*srcArea.h, "FBO -> texture");
    }
}

void FrameBuffer::bindScreen() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::FrameBuffer() : handle(0) {
    glGenFramebuffers(1, &handle);
    //LOG_OWN_ACTION(FBO_CREATION, 1, "");
}

FrameBuffer::FrameBuffer(FrameBuffer &&orig) : handle(orig.handle) {
    orig.handle = 0;
}

FrameBuffer::~FrameBuffer() {
    if (handle) {
        glDeleteFramebuffers(1, &handle);
        //LOG_OWN_ACTION(FBO_DELETION, 1, "");
    }
}

FrameBuffer &FrameBuffer::operator=(FrameBuffer &&orig) {
    if (this != &orig) {
        if (handle) {
            glDeleteFramebuffers(1, &handle);
            //LOG_OWN_ACTION(FBO_DELETION, 1, "");
        }
        handle = orig.handle;
        orig.handle = 0;
    }
    return *this;
}

FrameBuffer::operator bool() const {
    return handle != 0;
}

void FrameBuffer::setOutput(Texture2D *texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->handle, 0);
    PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::setOutput(RenderBuffer *renderBuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer->handle);
    PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::unsetOutput(Texture2D *texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::unsetOutput(RenderBuffer *renderBuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
    PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void FrameBuffer::unbind() {
    PEDANTIC_ONLY(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

}
