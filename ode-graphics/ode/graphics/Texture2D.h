
#pragma once

#include <ode/math/Vector2.h>
#include <ode/graphics/pixel-format.h>
#include <ode/graphics/FilterMode.h>
#include <ode/graphics/Bitmap.h>
#include <ode/graphics/BitmapConstRef.h>
#include "gl.h"

namespace ode {

class FrameBuffer;

/// Represents a 2D OpenGL texture
class Texture2D {
    friend class FrameBuffer;
    //DEBUG_INSTANCE_COUNTER(Texture2D)

public:
    explicit Texture2D(FilterMode filter = FilterMode::LINEAR, bool wrap = false);
    Texture2D(GLuint handle, PixelFormat format, const Vector2i &dimensions);
    Texture2D(const Texture2D &) = delete;
    Texture2D(Texture2D &&orig);
    ~Texture2D();
    Texture2D &operator=(const Texture2D &) = delete;
    /// Initializes the image data of the texture
    bool initialize(const BitmapConstRef &bitmap);
    /// Allocates the texture without setting its data
    bool initialize(PixelFormat format, const Vector2i &dimensions);
    /// Fills in a portion of the texture
    bool put(const Vector2i &position, const BitmapConstRef &bitmap);
    /// Returns true if image data has been initialized
    explicit operator bool() const;
    /// Binds the texture to the specified texture unit
    void bind(int unit) const;
    /// Unbinds the texture from the specified texture unit
    void unbind(int unit) const;
    /// Returns the dimensions of the zero-th mipmap level of the texture's image data
    Vector2i dimensions() const;
    /// Returns the texture's internal pixel format
    PixelFormat format() const;
    /// Transfers the zero-th mipmap level of the texture to a bitmap in physical memory
    Bitmap download() const;
    /// Automatically generates all mipmap levels for the texture
    void generateMipmaps(FilterMode filter = FilterMode::BILINEAR, bool wrap = false);
    /// Changes the texture's filtering mode
    void setFilterMode(FilterMode filter);

#ifdef __EMSCRIPTEN__
    /// Binds the texture in preparation for an external OpenGL call to modify its content
    void prepareExternalUpload();
    /// Ends and confirms the externally invoked OpenGL transaction and notifies the object about its new content dimensions and format
    void confirmExternalUpload(int width, int height, PixelFormat format);
#endif
    /// Returns the internal OpenGL handle
    GLuint getInternalGLHandle();

private:
    GLuint handle;
    Vector2i dims;
    PixelFormat fmt;
    bool hasMipmaps;
    size_t memorySize;

    bool initialize(PixelFormat format, const void *pixels, const Vector2i &dimensions);

};

}
