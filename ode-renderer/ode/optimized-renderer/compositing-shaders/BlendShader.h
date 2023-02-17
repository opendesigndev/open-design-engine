#pragma once

#include <octopus/octopus.h>
#include "CompositingShader.h"

namespace ode {

class BlendShader : public CompositingShader {

public:
    static constexpr int UNIT_DST = 0;
    static constexpr int UNIT_SRC = 1;

    static StringLiteral blendFunctionSource(octopus::BlendMode blendMode);

    BlendShader();
    bool initialize(const SharedResource &res, const StringLiteral &blendFunction);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &dstBounds, const ScaledBounds &srcBounds, bool ignoreSrcAlpha);

private:
    ShaderProgram shader;
    Uniform unifDstImage;
    Uniform unifSrcImage;
    Uniform unifIgnoreSrcAlpha;

};

}
