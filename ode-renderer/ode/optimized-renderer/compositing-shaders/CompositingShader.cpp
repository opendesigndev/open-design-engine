
#include "CompositingShader.h"

namespace ode {

CompositingShader::SharedResource::operator bool() const {
    return get() != nullptr;
}

CompositingShader::SharedResource CompositingShader::prepare() {
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VVARYING "vec2 texCoord[3];"
        "uniform mat2 vertexFraming;"
        "uniform mat2 texFraming[3];"
        "void main() {"
            "gl_Position = vec4(vertexFraming[0]+vertexFraming[1]*coord, 0.0, 1.0);"
            "texCoord[0] = texFraming[0][0]+texFraming[0][1]*coord;"
            "texCoord[1] = texFraming[1][0]+texFraming[1][1]*coord;"
            "texCoord[2] = texFraming[2][0]+texFraming[2][1]*coord;"
        "}\n"
    );
    SharedResource res;
    res.reset(new VertexShader("compositing-vs"));
    const GLchar *src[] = { ODE_COMPOSITING_SHADER_PREAMBLE, vsSrc.string };
    const GLint sln[] = { sizeof(ODE_COMPOSITING_SHADER_PREAMBLE)-1, vsSrc.length };
    if (!res->initialize(src, sln, sizeof(src)/sizeof(*src))) {
        res.reset(nullptr);
    }
    return res;
}

VertexShader *CompositingShader::getVertexShader(const SharedResource &res) {
    return res.get();
}

CompositingShader::CompositingShader() : initialized(false) { }

bool CompositingShader::initialize(ShaderProgram *shader) {
    if (!shader)
        return false;
    unifVertexFraming = shader->getUniform("vertexFraming");
    unifTexFraming[0] = shader->getUniform("texFraming[0]");
    unifTexFraming[1] = shader->getUniform("texFraming[1]");
    unifTexFraming[2] = shader->getUniform("texFraming[2]");
    return initialized = true;
}

void CompositingShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds) {
    ODE_ASSERT(viewport.b.x-viewport.a.x && viewport.b.y-viewport.a.y);
    float vertexFraming[4] = {
        float(2*(outputBounds.a.x-viewport.a.x)/(viewport.b.x-viewport.a.x)-1),
        float(2*(outputBounds.a.y-viewport.a.y)/(viewport.b.y-viewport.a.y)-1),
        float(2*(outputBounds.b.x-outputBounds.a.x)/(viewport.b.x-viewport.a.x)),
        float(2*(outputBounds.b.y-outputBounds.a.y)/(viewport.b.y-viewport.a.y)),
    };
    unifVertexFraming.setMat2(vertexFraming);
}

void CompositingShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds) {
    ODE_ASSERT(outputBounds);
    bind(viewport, outputBounds);
    float texFraming[4] = {
        float((outputBounds.a.x-input0Bounds.a.x)/(input0Bounds.b.x-input0Bounds.a.x)),
        float((outputBounds.a.y-input0Bounds.a.y)/(input0Bounds.b.y-input0Bounds.a.y)),
        float((outputBounds.b.x-outputBounds.a.x)/(input0Bounds.b.x-input0Bounds.a.x)),
        float((outputBounds.b.y-outputBounds.a.y)/(input0Bounds.b.y-input0Bounds.a.y)),
    };
    unifTexFraming[0].setMat2(texFraming);
}

void CompositingShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds, const ScaledBounds &input1Bounds) {
    ODE_ASSERT(outputBounds);
    bind(viewport, outputBounds, input0Bounds);
    float texFraming[4] = {
        float((outputBounds.a.x-input1Bounds.a.x)/(input1Bounds.b.x-input1Bounds.a.x)),
        float((outputBounds.a.y-input1Bounds.a.y)/(input1Bounds.b.y-input1Bounds.a.y)),
        float((outputBounds.b.x-outputBounds.a.x)/(input1Bounds.b.x-input1Bounds.a.x)),
        float((outputBounds.b.y-outputBounds.a.y)/(input1Bounds.b.y-input1Bounds.a.y)),
    };
    unifTexFraming[1].setMat2(texFraming);
}

void CompositingShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &input0Bounds, const ScaledBounds &input1Bounds, const ScaledBounds &input2Bounds) {
    ODE_ASSERT(outputBounds);
    bind(viewport, outputBounds, input0Bounds, input1Bounds);
    float texFraming[4] = {
        float((outputBounds.a.x-input2Bounds.a.x)/(input2Bounds.b.x-input2Bounds.a.x)),
        float((outputBounds.a.y-input2Bounds.a.y)/(input2Bounds.b.y-input2Bounds.a.y)),
        float((outputBounds.b.x-outputBounds.a.x)/(input2Bounds.b.x-input2Bounds.a.x)),
        float((outputBounds.b.y-outputBounds.a.y)/(input2Bounds.b.y-input2Bounds.a.y)),
    };
    unifTexFraming[2].setMat2(texFraming);
}

bool CompositingShader::ready() const {
    return initialized;
}

}
