
#include "ImageFillShader.h"

namespace ode {

ImageFillShader::ImageFillShader() : FillShader() { }

bool ImageFillShader::initialize(const SharedResource &res) {
    const StringLiteral fsSrc = ODE_STRLIT(
        ODE_GLSL_FVARYING "vec2 texCoord;"
        "uniform sampler2D image;"
        "void main() {"
            ODE_GLSL_FRAGCOLOR "=" ODE_GLSL_TEXTURE2D "(image, texCoord);"
        "}\n"
    );
    if (!res)
        return false;
    FragmentShader fs("fill-gradient");
    const GLchar *src[] = { ODE_FILL_SHADER_PREAMBLE, fsSrc.string };
    const GLint sln[] = { sizeof(ODE_FILL_SHADER_PREAMBLE)-1, fsSrc.length };
    if (!fs.initialize(src, sln, sizeof(src)/sizeof(*src)))
        return false;
    if (!shader.initialize(getVertexShader(res), &fs))
        return false;
    unifImage = shader.getUniform("image");
    shader.bind();
    unifImage.setInt(UNIT_IMAGE);
    return FillShader::initialize(&shader);
}

void ImageFillShader::bind(const PixelBounds &viewport, const ScaledBounds &outputBounds, const Matrix3x3f &fragTransform) {
    shader.bind();
    FillShader::bind(viewport, outputBounds, fragTransform);
}

}
