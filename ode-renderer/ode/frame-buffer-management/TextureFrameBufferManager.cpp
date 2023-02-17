
#include "TextureFrameBufferManager.h"

namespace ode {

bool TextureFrameBufferManager::DimsCmp::operator()(const Vector2i &a, const Vector2i &b) const {
    return a.y < b.y || (a.y == b.y && a.x < b.x);
}

TextureFrameBufferPtr TextureFrameBufferManager::acquire(PixelBounds &bounds) {
    Vector2i dimensions = bounds.dimensions();
    dimensions.x = (dimensions.x+0xff)&~0xff;
    dimensions.y = (dimensions.y+0xff)&~0xff;
    bounds.b = bounds.a+dimensions;
    return acquireExact(bounds);
}

TextureFrameBufferPtr TextureFrameBufferManager::acquireExact(const PixelBounds &bounds) {
    std::map<Vector2i, std::vector<TextureFrameBuffer>, DimsCmp>::iterator it = stock.find(bounds.dimensions());
    if (it == stock.end() || it->second.empty()) {
        TextureFrameBufferPtr result(new TextureFrameBuffer(this));
        if (!result->initialize(bounds.dimensions()))
            return nullptr;
        return result;
    } else {
        TextureFrameBufferPtr result(new TextureFrameBuffer((TextureFrameBuffer &&) it->second.back(), this));
        it->second.pop_back();
        return result;
    }
}

void TextureFrameBufferManager::relinquish(TextureFrameBuffer &&obj) {
    stock[obj.dimensions()].emplace_back((TextureFrameBuffer &&) obj, nullptr);
}

}
