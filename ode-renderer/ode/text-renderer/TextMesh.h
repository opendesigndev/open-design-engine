
#pragma once

#include <vector>
#include <ode-graphics.h>
#include <ode-logic.h>

namespace ode {

class TextMesh : public TextShapeHolder::RendererData {

public:

private:
    struct FontSegment {
        int start, length;
        Texture2D *texture;
    };

    Mesh mesh;
    std::vector<FontSegment> segments;

};

}
