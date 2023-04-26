
#include "ImageTextShader.h"

namespace ode {

ImageTextShader::ImageTextShader() = default;

bool ImageTextShader::initialize() {
    const char *const attribs[] = {
        "coord",
        "texCoord",
        nullptr
    };
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VATTRIB "vec2 texCoord;"
        ODE_GLSL_VVARYING "vec2 vTexCoord;"
        "uniform mat3 transformation;"
        "uniform vec2 texCoordFactor;"
        "void main() {"
            "vec3 position = transformation*vec3(coord, 1.0);"
            "gl_Position = vec4(position.xy, 0.0, position.z);"
            "vTexCoord = texCoordFactor*texCoord;"
        "}\n"
    );
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 vTexCoord;"
        "uniform sampler2D image;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "=" ODE_GLSL_TEXTURE2D "(image, vTexCoord).bgra;"
        "}\n"
    );
    VertexShader vs("imgtext-vs");
    const GLchar *vsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, vsSrc.string };
    const GLint vsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, vsSrc.length };
    FragmentShader fs("imgtext-fs");
    const GLchar *fsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, fsSrc.string };
    const GLint fsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!vs.initialize(vsrc, vsln, sizeof(vsrc)/sizeof(*vsrc)))
        return false;
    if (!fs.initialize(fsrc, fsln, sizeof(fsrc)/sizeof(*fsrc)))
        return false;
    if (!shader.initialize(&vs, &fs, attribs))
        return false;
    unifTransformation = shader.getUniform("transformation");
    unifImage = shader.getUniform("image");
    unifTexCoordFactor = shader.getUniform("texCoordFactor");
    shader.bind();
    unifImage.setInt(UNIT_IN);
    float defaultFactor[2] = { 1.f, 1.f };
    unifTexCoordFactor.setVec2(defaultFactor);
    return true;
}

bool ImageTextShader::ready() const {
    return (bool) shader;
}

void ImageTextShader::bind(const PixelBounds &viewport, const Matrix3x3f &transformation) {
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
    unifTransformation.setMat3(m);
}

Uniform &ImageTextShader::texCoordFactorUniform() {
    return unifTexCoordFactor;
}

}
