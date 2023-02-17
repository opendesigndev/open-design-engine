
#include "EffectShader.h"

namespace ode {

EffectShader::SharedResource::operator bool() const {
    return get() != nullptr;
}

EffectShader::SharedResource EffectShader::prepare() {
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VVARYING "vec2 texCoord;"
        "uniform mat2 vertexFraming;"
        "uniform mat2 texFraming;"
        "void main() {"
            "gl_Position = vec4(vertexFraming[0]+vertexFraming[1]*coord, 0.0, 1.0);"
            "texCoord = texFraming[0]+texFraming[1]*coord;"
        "}\n"
    );
    SharedResource res;
    res.reset(new VertexShader("effect-vs"));
    const GLchar *src[] = { ODE_EFFECT_SHADER_PREAMBLE, vsSrc.string };
    const GLint sln[] = { sizeof(ODE_EFFECT_SHADER_PREAMBLE)-1, vsSrc.length };
    if (!res->initialize(src, sln, sizeof(src)/sizeof(*src))) {
        res.reset(nullptr);
    }
    return res;
}

VertexShader *EffectShader::getVertexShader(const SharedResource &res) {
    return res.get();
}

EffectShader::EffectShader() : initialized(false) { }

bool EffectShader::initialize(ShaderProgram *shader) {
    if (!shader)
        return false;
    unifVertexFraming = shader->getUniform("vertexFraming");
    unifTexFraming = shader->getUniform("texFraming");
    return initialized = true;
}

void EffectShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const ScaledBounds &inputBounds) {
    ODE_ASSERT(viewport && outputBounds);
    float vertexFraming[4] = {
        float(2*(outputBounds.a.x-viewport.a.x)/(viewport.b.x-viewport.a.x)-1),
        float(2*(outputBounds.a.y-viewport.a.y)/(viewport.b.y-viewport.a.y)-1),
        float(2*(outputBounds.b.x-outputBounds.a.x)/(viewport.b.x-viewport.a.x)),
        float(2*(outputBounds.b.y-outputBounds.a.y)/(viewport.b.y-viewport.a.y)),
    };
    unifVertexFraming.setMat2(vertexFraming);
    float texFraming[4] = {
        float((outputBounds.a.x-inputBounds.a.x)/(inputBounds.b.x-inputBounds.a.x)),
        float((outputBounds.a.y-inputBounds.a.y)/(inputBounds.b.y-inputBounds.a.y)),
        float((outputBounds.b.x-outputBounds.a.x)/(inputBounds.b.x-inputBounds.a.x)),
        float((outputBounds.b.y-outputBounds.a.y)/(inputBounds.b.y-inputBounds.a.y)),
    };
    unifTexFraming.setMat2(texFraming);
}

bool EffectShader::ready() const {
    return initialized;
}

}
