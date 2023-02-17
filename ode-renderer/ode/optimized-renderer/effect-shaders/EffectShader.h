
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

#define ODE_EFFECT_SHADER_PREAMBLE ODE_GLOBAL_SHADER_PREAMBLE

namespace ode {

class EffectShader {

public:
    static constexpr int UNIT_BASIS = 0;

    class SharedResource : private std::unique_ptr<VertexShader> {
        friend class EffectShader;

    public:
        explicit operator bool() const;

    };

    static SharedResource prepare();

    EffectShader(const EffectShader &) = delete;
    virtual ~EffectShader() = default;
    EffectShader &operator=(const EffectShader &) = delete;
    bool ready() const;

protected:
    static VertexShader *getVertexShader(const SharedResource &res);

    EffectShader();
    bool initialize(ShaderProgram *shader);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds);

private:
    bool initialized;
    Uniform unifVertexFraming;
    Uniform unifTexFraming;

};

}
