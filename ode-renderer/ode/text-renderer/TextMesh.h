
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
    enum SegmentType {
        SDF,
        MSDF,
        IMAGE
    };

    static std::unique_ptr<TextMesh> build(TextRenderer *parent, odtr::TextShapeHandle handle);

    void draw(Uniform &vec2TexCoordFactor, int textureUnit, SegmentType type) const;

private:
    struct FontSegment {
        int start, length;
        SegmentType type;
        const Texture2D *texture;
    };

    Mesh mesh;
    std::vector<FontSegment> segments;

};

}

#endif
