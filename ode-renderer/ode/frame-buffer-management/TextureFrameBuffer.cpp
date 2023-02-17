
#include "TextureFrameBuffer.h"

#include "TextureFrameBufferManager.h"

namespace ode {

TextureFrameBuffer::TextureFrameBuffer(TextureFrameBufferManager *parent) : parent(parent) { }

TextureFrameBuffer::TextureFrameBuffer(TextureFrameBuffer &&orig, TextureFrameBufferManager *parent) : Texture2D((Texture2D &&) orig), parent(parent), frameBuffer((FrameBuffer &&) orig.frameBuffer) { }

TextureFrameBuffer::~TextureFrameBuffer() {
    if (parent)
        parent->relinquish((TextureFrameBuffer &&) *this);
}

bool TextureFrameBuffer::initialize(const Vector2i &dimensions) {
    if (!Texture2D::initialize(nullptr, dimensions.x, dimensions.y, PixelFormat::RGBA)) // TODO dims as single argument
        return false;
    frameBuffer.setOutput(this);
    return true;
}

void TextureFrameBuffer::bind(int unit) const {
    Texture2D::bind(unit);
}

void TextureFrameBuffer::bind() {
    frameBuffer.bind();
}

void TextureFrameBuffer::unbind() {
    frameBuffer.unbind();
}

}
