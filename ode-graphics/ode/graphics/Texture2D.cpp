
#include "Texture2D.h"

#include "GraphicsContext.h"
#include "FrameBuffer.h"

#ifndef GL_ALPHA32F
#define GL_ALPHA32F GL_ALPHA32F_ARB
#endif

namespace ode {

int DEBUG_TEXTURES_CREATED = 0;

static bool isPOT(int x) {
    return !(x&x-1);
}

// Note: In WebGL 1, the internal format is inferred from the input pixel format
void convertPixelFormat(PixelFormat format, GLint &internalFormat, GLenum &pixelFormat, GLenum &pixelType) {
    if (format&PIXEL_FLOAT_BIT) {
        switch (pixelChannels(format)) {
            case 1:
                /*if (format&PIXEL_ALPHA_ONLY_BIT) {
                    #ifdef ODE_WEBGL_COMPATIBILITY
                        internalFormat = GL_ALPHA;
                    #else
                        internalFormat = GL_ALPHA32F;
                    #endif
                    pixelFormat = GL_ALPHA;
                    pixelType = GL_FLOAT;
                    break;
                }*/
                ODE_ASSERT(!(format&PIXEL_ALPHA_ONLY_BIT)); // unsupported
                #ifdef ODE_WEBGL_COMPATIBILITY
                    internalFormat = GL_LUMINANCE;
                #else
                    internalFormat = GL_R32F;
                #endif
                #ifdef USE_OPENGL3_CORE
                    pixelFormat = GL_RED;
                #else
                    pixelFormat = GL_LUMINANCE;
                #endif
                pixelType = GL_FLOAT;
                break;
            case 3:
                #ifdef ODE_WEBGL_COMPATIBILITY
                    internalFormat = GL_RGB;
                #else
                    internalFormat = GL_RGB32F;
                #endif
                pixelFormat = GL_RGB;
                pixelType = GL_FLOAT;
                break;
            case 4:
                #ifdef ODE_WEBGL_COMPATIBILITY
                    internalFormat = GL_RGBA;
                #else
                    internalFormat = GL_RGBA32F;
                #endif
                pixelFormat = GL_RGBA;
                pixelType = GL_FLOAT;
                break;
            default:
                ODE_ASSERT(!"Unexpected pixel format");
        }
    } else {
        switch (pixelChannels(format)) {
            case 1:
                /*if (format&PIXEL_ALPHA_ONLY_BIT) {
                    internalFormat = GL_ALPHA;
                    pixelFormat = GL_ALPHA;
                    pixelType = GL_UNSIGNED_BYTE;
                    break;
                }*/
                ODE_ASSERT(!(format&PIXEL_ALPHA_ONLY_BIT)); // unsupported
                #ifdef ODE_WEBGL_COMPATIBILITY
                    internalFormat = GL_LUMINANCE;
                    pixelFormat = GL_LUMINANCE;
                #else
                    internalFormat = GL_R8;
                    pixelFormat = GL_RED;
                #endif
                pixelType = GL_UNSIGNED_BYTE;
                break;
            case 3:
                internalFormat = GL_RGB;
                pixelFormat = GL_RGB;
                pixelType = GL_UNSIGNED_BYTE;
                break;
            case 4:
                internalFormat = GL_RGBA;
                pixelFormat = GL_RGBA;
                pixelType = GL_UNSIGNED_BYTE;
                break;
            default:
                ODE_ASSERT(!"Unexpected pixel format");
        }
    }
}

GLint convertFilterMode(FilterMode mode) {
    switch (mode) {
        case FilterMode::NEAREST:
            return GL_NEAREST;
        case FilterMode::LINEAR:
            return GL_LINEAR;
        case FilterMode::BILINEAR:
            return GL_LINEAR_MIPMAP_NEAREST;
        case FilterMode::TRILINEAR:
            return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

Texture2D::Texture2D(FilterMode filter, bool wrap) : handle(0), fmt(), hasMipmaps(false), memorySize(0) {
    glGenTextures(1, &handle);
    ++DEBUG_TEXTURES_CREATED;
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == FilterMode::NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == FilterMode::NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    ODE_CHECK_GL_ERROR();
    //LOG_OWN_ACTION(TEXTURE_CREATION, 2, "");
}

Texture2D::Texture2D(GLuint handle, PixelFormat format, const Vector2i &dimensions) : handle(handle), dims(dimensions), fmt(format), hasMipmaps(false), memorySize(0) {
    memorySize = pixelSize(format)*dims.x*dims.y;
    //MemoryWatch::instance().registerChange((long long) memorySize, true);
}

Texture2D::Texture2D(Texture2D &&orig) : handle(orig.handle), dims(orig.dims), fmt(orig.fmt), hasMipmaps(orig.hasMipmaps), memorySize(orig.memorySize) {
    orig.handle = 0;
    orig.memorySize = 0;
}

Texture2D::~Texture2D() {
    if (handle) {
        glDeleteTextures(1, &handle);
        ODE_CHECK_GL_ERROR();
        //MemoryWatch::instance().registerChange(-(long long) memorySize_, true);
        //LOG_OWN_ACTION(TEXTURE_DELETION, (int) memorySize_, "");
    }
}

bool Texture2D::initialize(const void *pixels, int width, int height, PixelFormat format) {
    if (!(width > 0 && height > 0 && width <= GraphicsContext::getMaxTextureSize() && height <= GraphicsContext::getMaxTextureSize()))
        return false;
    dims = Vector2i(width, height);
    fmt = format;
    GLint internalFormat = GL_INVALID_INDEX;
    GLenum pixelFormat = GL_INVALID_INDEX;
    GLenum pixelType = GL_INVALID_INDEX;
    convertPixelFormat(format, internalFormat, pixelFormat, pixelType);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, pixelFormat, pixelType, pixels);
    ODE_CHECK_GL_ERROR();
    size_t newMemSize = pixelSize(format)*dims.x*dims.y;
    //MemoryWatch::instance().registerChange((long long) newMemSize-(long long) memorySize_, true);
    hasMipmaps = false;
    memorySize = newMemSize;
    //LOG_OWN_ACTION(TEXTURE_UPLOAD, (int) memorySize_, "");
    // Texture must still be bound after return!
    return true;
}

bool Texture2D::initialize(const BitmapConstRef &bitmap) {
    return initialize(bitmap.pixels, bitmap.width(), bitmap.height(), bitmap.format);
}

Texture2D::operator bool() const {
    return handle != 0;
}

void Texture2D::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, handle);
    ODE_CHECK_GL_ERROR();
}

void Texture2D::unbind(int unit) const {
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, 0);
    ODE_CHECK_GL_ERROR();
}

Vector2i Texture2D::dimensions() const {
    return dims;
}

PixelFormat Texture2D::format() const {
    return fmt;
}

Bitmap Texture2D::download() const {
    ODE_ASSERT(handle && fmt != PixelFormat::EMPTY);
    ODE_ASSERT(fmt == PixelFormat::RGBA || fmt == PixelFormat::PREMULTIPLIED_RGBA); // otherwise won't work in WebGL 1 (?)
    #ifdef ODE_WEBGL_COMPATIBILITY // TODO implement this using intermediate draw buffer?
        if (!(fmt == PixelFormat::R || fmt == PixelFormat::RGB || fmt == PixelFormat::RGBA || fmt == PixelFormat::PREMULTIPLIED_RGBA))
            return Bitmap();
    #endif
    Bitmap bitmap(fmt, dims);
    if (!bitmap)
        return Bitmap();
    #ifndef ODE_WEBGL_COMPATIBILITY
        glBindTexture(GL_TEXTURE_2D, handle);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *) bitmap);
    #else
        FrameBuffer fb;
        fb.setOutput(const_cast<Texture2D *>(this));
        fb.bind(); // !!! what if something is already bound and needs to be restored???
        glViewport(0, 0, dims.x, dims.y);
        glReadPixels(0, 0, dims.x, dims.y, GL_RGBA, GL_UNSIGNED_BYTE, (void *) bitmap);
        fb.unbind();
        fb.unsetOutput(const_cast<Texture2D*>(this));
    #endif
    ODE_CHECK_GL_ERROR();
    //LOG_OWN_ACTION(TEXTURE_DOWNLOAD, (int) memorySize_, "");
    return bitmap;
}

void Texture2D::generateMipmaps(FilterMode filter, bool wrap) {
    if (hasMipmaps)
        return;
#ifdef ODE_WEBGL_COMPATIBILITY
    if (!(isPOT(dims.x) && isPOT(dims.y))) {
        ODE_ASSERT(!"Unsupported");
        return;
    }
#endif
    glBindTexture(GL_TEXTURE_2D, handle);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertFilterMode(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
#ifndef ODE_WEBGL_COMPATIBILITY
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -.5f);
#endif
    ODE_CHECK_GL_ERROR();
    size_t newMemSize = (size_t) (4./3.*memorySize);
    //MemoryWatch::instance().registerChange((long long) newMemSize-(long long) memorySize_, true);
    hasMipmaps = true;
    memorySize = newMemSize;
}

void Texture2D::setFilterMode(FilterMode filter) {
    if (!hasMipmaps && filter >= FilterMode::BILINEAR)
        return; // log this as warning?
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == FilterMode::NEAREST ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, convertFilterMode(filter));
    ODE_CHECK_GL_ERROR();
}

#ifdef __EMSCRIPTEN__
void Texture2D::prepareExternalUpload() {
    glBindTexture(GL_TEXTURE_2D, handle);
    ODE_CHECK_GL_ERROR();
}

void Texture2D::confirmExternalUpload(int width, int height, PixelFormat format) {
    dims.x = width;
    dims.y = height;
    fmt = format;
    size_t newMemSize = pixelSize(format)*width*height;
    //MemoryWatch::instance().registerChange((long long) newMemSize-(long long) memorySize_, true);
    memorySize = newMemSize;
    //LOG_ACTION(TEXTURE_UPLOAD, nullptr, (int) memorySize_, "external");
}
#endif

GLuint Texture2D::getInternalGLHandle() {
    return handle;
}

}
