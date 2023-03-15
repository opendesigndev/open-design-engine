
#include "TransformShader.h"

namespace ode {

TransformShader::TransformShader() = default;

bool TransformShader::initialize() {
    const StringLiteral vsSrc = ODE_STRLIT(
        ODE_GLSL_VATTRIB "vec2 coord;"
        ODE_GLSL_VVARYING "vec2 texCoord;"
        "uniform mat3 transformation;"
        "void main() {"
            "vec3 position = transformation*vec3(2.0*coord-1.0, 1.0);"
            "gl_Position = vec4(position.xy, 0.0, position.z);"
            "texCoord = coord;"
        "}\n"
    );
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D image;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "= " ODE_GLSL_TEXTURE2D "(image, texCoord);"
        "}\n"
    );
    VertexShader vs("transform-fs");
    const GLchar *vsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, vsSrc.string };
    const GLint vsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, vsSrc.length };
    FragmentShader fs("transform-fs");
    const GLchar *fsrc[] = { ODE_GLOBAL_SHADER_PREAMBLE, fsSrc.string };
    const GLint fsln[] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!vs.initialize(vsrc, vsln, sizeof(vsrc)/sizeof(*vsrc)))
        return false;
    if (!fs.initialize(fsrc, fsln, sizeof(fsrc)/sizeof(*fsrc)))
        return false;
    if (!shader.initialize(&vs, &fs))
        return false;
    unifTransformation = shader.getUniform("transformation");
    unifImage = shader.getUniform("image");
    shader.bind();
    unifImage.setInt(UNIT_IN);
    return true;
}

bool TransformShader::ready() const {
    return (bool) shader;
}

void TransformShader::bind(const Matrix3x3f &transformation, const Vector2i &outputSize) {
    shader.bind();
    float m[9] = {
        transformation[0][0], transformation[0][1], transformation[0][2],
        transformation[1][0], transformation[1][1], transformation[1][2],
        transformation[2][0], transformation[2][1], transformation[2][2],
    };
    unifTransformation.setMat3(m);
}

}
