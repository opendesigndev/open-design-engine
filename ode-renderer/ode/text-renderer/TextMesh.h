
#pragma once

#ifdef ODE_REALTIME_TEXT_RENDERER

#include <vector>
#include <memory>
#include <open-design-text-renderer/text-renderer-api.h>
#include <ode-graphics.h>
#include <ode-logic.h>

namespace ode {

class TextRenderer;

class TextMesh : public TextShapeHolder::RendererData {

public:
    static std::unique_ptr<TextMesh> build(TextRenderer *parent, odtr::TextShapeHandle handle);

    void draw(Uniform &vec2TexCoordFactor, int textureUnit) const;

private:
    struct FontSegment {
        int start, length;
        const Texture2D *texture;
    };

    Mesh mesh;
    std::vector<FontSegment> segments;

};

}

#endif
