
#include "FillShader.h"

namespace ode {

FillShader::SharedResource::operator bool() const {
    return get() != nullptr;
}

FillShader::SharedResource FillShader::prepare() {
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VVARYING "vec2 texCoord;"
        "uniform mat2 framing;"
        "uniform mat3 fragTransform;"
        "void main() {"
            "gl_Position = vec4(framing[0]+framing[1]*coord, 0.0, 1.0);"
            "texCoord = (fragTransform*vec3(coord, 1.0)).xy;"
        "}\n"
    );
    SharedResource res;
    res.reset(new VertexShader("fill-vs"));
    const GLchar *src[] = { ODE_FILL_SHADER_PREAMBLE, vsSrc.string };
    const GLint sln[] = { sizeof(ODE_FILL_SHADER_PREAMBLE)-1, vsSrc.length };
    if (!res->initialize(src, sln, sizeof(src)/sizeof(*src))) {
        res.reset(nullptr);
    }
    return res;
}

VertexShader *FillShader::getVertexShader(const SharedResource &res) {
    return res.get();
}

FillShader::FillShader() : initialized(false) { }

bool FillShader::initialize(ShaderProgram *shader) {
    if (!shader)
        return false;
    unifFraming = shader->getUniform("framing");
    unifFragTransform = shader->getUniform("fragTransform");
    return initialized = true;
}

void FillShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform) {
    ODE_ASSERT(viewport);
    float vertexFraming[4] = {
        float(2*(outputBounds.a.x-viewport.a.x)/(viewport.b.x-viewport.a.x)-1),
        float(2*(outputBounds.a.y-viewport.a.y)/(viewport.b.y-viewport.a.y)-1),
        float(2*(outputBounds.b.x-outputBounds.a.x)/(viewport.b.x-viewport.a.x)),
        float(2*(outputBounds.b.y-outputBounds.a.y)/(viewport.b.y-viewport.a.y)),
    };
    Matrix3x3f invFragTransform = inverse(fragTransform)*Matrix3x3f(
        float(outputBounds.b.x-outputBounds.a.x), 0.f, 0.f,
        0.f, float(outputBounds.b.y-outputBounds.a.y), 0.f,
        float(outputBounds.a.x), float(outputBounds.a.y), 1.f
    );
    float m[3*3] = {
        invFragTransform[0][0], invFragTransform[0][1], invFragTransform[0][2],
        invFragTransform[1][0], invFragTransform[1][1], invFragTransform[1][2],
        invFragTransform[2][0], invFragTransform[2][1], invFragTransform[2][2],
    };
    unifFraming.setMat2(vertexFraming);
    unifFragTransform.setMat3(m);
}

bool FillShader::ready() const {
    return initialized;
}

}
