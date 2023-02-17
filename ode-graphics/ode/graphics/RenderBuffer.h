
#pragma once

#include <ode/math/Vector2.h>
#include "gl.h"

namespace ode {

class Texture2D;
class FrameBuffer;

/// Represents an OpenGL render buffer object
class RenderBuffer {
    friend class Texture2D;
    friend class FrameBuffer;
    //DEBUG_INSTANCE_COUNTER(RenderBuffer)

public:
    explicit RenderBuffer(const Vector2i &dimensions);
    RenderBuffer(const RenderBuffer &) = delete;
    RenderBuffer(RenderBuffer &&orig);
    ~RenderBuffer();
    RenderBuffer &operator=(const RenderBuffer&) = delete;
    RenderBuffer &operator=(RenderBuffer&& orig);
    explicit operator bool() const;
    Vector2i dimensions() const;

private:
    GLuint handle;
    Vector2i dims;

};

}
