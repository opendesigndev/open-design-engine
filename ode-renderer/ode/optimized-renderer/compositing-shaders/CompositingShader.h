
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

#define ODE_COMPOSITING_SHADER_PREAMBLE ODE_GLOBAL_SHADER_PREAMBLE

namespace ode {

class CompositingShader {

public:
    class SharedResource : private std::unique_ptr<VertexShader> {
        friend class CompositingShader;

    public:
        explicit operator bool() const;

    };

    static SharedResource prepare();

    CompositingShader(const CompositingShader &) = delete;
    virtual ~CompositingShader() = default;
    CompositingShader &operator=(const CompositingShader &) = delete;
    bool ready() const;

protected:
    static VertexShader *getVertexShader(const SharedResource &res);

    CompositingShader();
    bool initialize(ShaderProgram *shader);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds, const ScaledBounds &input1Bounds);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds, const ScaledBounds &input1Bounds, const ScaledBounds &input2Bounds);

private:
    bool initialized;
    Uniform unifVertexFraming;
    Uniform unifTexFraming[3];

};

}
