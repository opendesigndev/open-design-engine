
#include "SDFTextShader.h"

namespace ode {

SDFTextShader::SDFTextShader() = default;

bool SDFTextShader::initialize() {
    const char *const attribs[] = {
        "coord",
        "texCoord",
        "color",
        "outputRange",
        nullptr
    };
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VATTRIB "vec2 texCoord;"
        ODE_GLSL_VATTRIB "vec4 color;"
        ODE_GLSL_VATTRIB "float outputRange;"
        ODE_GLSL_VVARYING "vec2 vTexCoord;"
        ODE_GLSL_VVARYING "vec4 vColor;" // may be flat
        ODE_GLSL_VVARYING "float screenPxRange;" // may be flat
        "uniform mat3 transformation;"
        "uniform vec2 texCoordFactor;"
        "uniform float rangeFactor;"
        "void main() {"
            "vec3 position = transformation*vec3(coord, 1.0);"
            "gl_Position = vec4(position.xy, 0.0, position.z);"
            "vTexCoord = texCoordFactor*texCoord;"
            "vColor = color;"
            "screenPxRange = rangeFactor*outputRange;"
        "}\n"
    );
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 vTexCoord;"
        ODE_GLSL_FVARYING "vec4 vColor;" // may be flat
        ODE_GLSL_FVARYING "float screenPxRange;" // may be flat
        "uniform sampler2D sdf;"
        "float median(vec3 x) {"
            "return max(min(x.r, x.g), min(max(x.r, x.g), x.b));"
        "}"
        "void main() {"
            "float sd = median(" ODE_GLSL_TEXTURE2D "(sdf, vTexCoord).rgb)-0.5;"
            "float alpha = clamp(screenPxRange*sd+0.5, 0.0, 1.0);"
            ODE_GLSL_FRAGCOLOR "= alpha*vColor;"
        "}\n"
    );
    VertexShader vs("msdf-vs");
    const GLchar *vsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, vsSrc.string };
    const GLint vsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, vsSrc.length };
    FragmentShader fs("msdf-fs");
    const GLchar *fsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, fsSrc.string };
    const GLint fsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!vs.initialize(vsrc, vsln, sizeof(vsrc)/sizeof(*vsrc)))
        return false;
    if (!fs.initialize(fsrc, fsln, sizeof(fsrc)/sizeof(*fsrc)))
        return false;
    if (!shader.initialize(&vs, &fs, attribs))
        return false;
    unifTransformation = shader.getUniform("transformation");
    unifSDF = shader.getUniform("sdf");
    unifTexCoordFactor = shader.getUniform("texCoordFactor");
    unifRangeFactor = shader.getUniform("rangeFactor");
    shader.bind();
    unifSDF.setInt(UNIT_IN);
    float defaultFactor[2] = { 1.f, 1.f };
    unifTexCoordFactor.setVec2(defaultFactor);
    return true;
}

bool SDFTextShader::ready() const {
    return (bool) shader;
}

void SDFTextShader::bind(const PixelBounds &viewport, const Matrix3x3f &transformation) {
    shader.bind();
    Matrix3x3f projectTransform = Matrix3x3f(
        float(2./viewport.dimensions().x), 0.f, 0.f,
        0.f, float(2./viewport.dimensions().y), 0.f,
        float(-1-2.*viewport.a.x/viewport.dimensions().x), float(-1-2.*viewport.a.y/viewport.dimensions().y), 1.f
    )*transformation;
    float m[9] = {
        projectTransform[0][0], projectTransform[0][1], projectTransform[0][2],
        projectTransform[1][0], projectTransform[1][1], projectTransform[1][2],
        projectTransform[2][0], projectTransform[2][1], projectTransform[2][2],
    };
    float hScale = (transformation*Vector3f(1.f, 0.f, 0.f)).length();
    float vScale = (transformation*Vector3f(0.f, 1.f, 0.f)).length();
    unifTransformation.setMat3(m);
    unifRangeFactor.setFloat(.5f*(hScale+vScale));
}

Uniform &SDFTextShader::texCoordFactorUniform() {
    return unifTexCoordFactor;
}

}
