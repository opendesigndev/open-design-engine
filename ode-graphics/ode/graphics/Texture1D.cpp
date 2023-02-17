
#include "Texture1D.h"

#include "GraphicsContext.h"

namespace ode {

// Implementation in Texture2D.cpp
void convertPixelFormat(PixelFormat format, GLint &internalFormat, GLenum &pixelFormat, GLenum &pixelType);

// 1D textures not supported in WebGL 1
#ifdef __EMSCRIPTEN__
    #undef GL_TEXTURE_1D
    #define GL_TEXTURE_1D GL_TEXTURE_2D
    #define glTexImage1D(t, lvl, ifmt, w, b, fmt, pt, p) glTexImage2D(t, lvl, ifmt, w, 1, b, fmt, pt, p)
#endif

Texture1D::Texture1D(FilterMode filter, bool wrap) : handle(0), width(0), memorySize(0) {
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_1D, handle);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, filter == FilterMode::NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, filter == FilterMode::NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_1D, 0);
    ODE_CHECK_GL_ERROR();
    //LOG_OWN_ACTION(TEXTURE_CREATION, 1, "");
}

Texture1D::~Texture1D() {
    if (handle) {
        glDeleteTextures(1, &handle);
        ODE_CHECK_GL_ERROR();
        //MemoryWatch::instance().registerChange(-(long long) memorySize_, true);
        //LOG_OWN_ACTION(TEXTURE_DELETION, (int) memorySize_, "");
    }
}

bool Texture1D::initialize(const void *pixels, int width, PixelFormat format) {
    if (width > GraphicsContext::getMaxTextureSize())
        return false;
    this->width = width;
    GLint internalFormat = GL_INVALID_INDEX;
    GLenum pixelFormat = GL_INVALID_INDEX;
    GLenum pixelType = GL_INVALID_INDEX;
    convertPixelFormat(format, internalFormat, pixelFormat, pixelType);
    glBindTexture(GL_TEXTURE_1D, handle);
    glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, width, 0, pixelFormat, pixelType, pixels);
    ODE_CHECK_GL_ERROR();
    size_t newMemSize = pixelSize(format)*width;
    //MemoryWatch::instance().registerChange((long long) newMemSize-(long long) memorySize_, true);
    memorySize = newMemSize;
    //LOG_OWN_ACTION(TEXTURE_UPLOAD, (int) memorySize_, "");
    return true;
}

void Texture1D::bind(int slot) const {
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_1D, handle);
    ODE_CHECK_GL_ERROR();
}

}
