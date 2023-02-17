
#pragma once

#include <memory>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode-logic.h>

#define ODE_FILL_SHADER_PREAMBLE ODE_GLOBAL_SHADER_PREAMBLE

namespace ode {

class FillShader {

public:
    class SharedResource : private std::unique_ptr<VertexShader> {
        friend class FillShader;

    public:
        explicit operator bool() const;

    };

    static SharedResource prepare();

    FillShader(const FillShader &) = delete;
    virtual ~FillShader() = default;
    FillShader &operator=(const FillShader &) = delete;
    bool ready() const;

protected:
    static VertexShader *getVertexShader(const SharedResource &res);

    FillShader();
    bool initialize(ShaderProgram *shader);
    void bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform);

private:
    bool initialized;
    Uniform unifFraming;
    Uniform unifFragTransform;

};

}
