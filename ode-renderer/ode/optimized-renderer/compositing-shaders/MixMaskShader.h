
#pragma once

#include <ode-logic.h>
#include "CompositingShader.h"

namespace ode {

class MixMaskShader : public CompositingShader {

public:
    static constexpr int UNIT_A = 0;
    static constexpr int UNIT_B = 1;
    static constexpr int UNIT_MASK = 2;

    MixMaskShader();
    bool initialize(const SharedResource &res);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &aBounds, const ScaledBounds &bBounds, const ScaledBounds &maskBounds, const ChannelMatrix &channelMatrix);

private:
    ShaderProgram shader;
    Uniform unifAImage;
    Uniform unifBImage;
    Uniform unifMaskImage;
    Uniform unifMaskChannelFactor;
    Uniform unifMaskBias;

};

}
