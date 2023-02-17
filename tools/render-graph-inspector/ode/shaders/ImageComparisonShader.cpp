
#include "ImageComparisonShader.h"

#include <string>
#include <sstream>

namespace {
std::string getDiffFragmentShaderBody() {
    return
        "   gl_FragColor = abs(colL-colR) * diffWeight;\n"
        "   gl_FragColor.a = 1.0;\n";
}
std::string getSliderFragmentShaderBody() {
    return
        "   if (abs(srcCoord.x - sliderXPos) < sliderStrokeWidth) {\n"
        "       gl_FragColor = vec4(0.2, 0.2, 0.2, 1.0);\n"
        "   } else if (srcCoord.x < sliderXPos) {\n"
        "       gl_FragColor = colL;\n"
        "   } else {\n"
        "       gl_FragColor = colR;\n"
        "   }\n"
        "   if (ignoreAlpha) {\n"
        "       gl_FragColor.a = 1.0;\n"
        "   }\n";
}
std::string getFadeFragmentShaderBody() {
    return
        "   if (colL.a <= 0.05) {\n"
        "       gl_FragColor = colR;\n"
        "   } else if (colR.a <= 0.05) {\n"
        "       gl_FragColor = colL;\n"
        "   } else {\n"
        "       gl_FragColor = (colL+colR)/2;\n"
        "   }\n"
        "   if (ignoreAlpha) {\n"
        "       gl_FragColor.a = 1.0;\n"
        "   }\n";
}
std::string getFragmentShader(ImageComparisonShader::Type type) {
    // TODO: Matus: Replace the manual checking of bounds by using some other way?
    std::stringstream ss;
    ss <<
        "varying vec2 srcCoord;\n"
        "uniform sampler2D srcImageL;\n"
        "uniform sampler2D srcImageR;\n"
        "uniform mat2 imageLReframing;\n"
        "uniform mat2 imageRReframing;\n"
        "uniform float sliderXPos;\n"
        "uniform float sliderStrokeWidth;\n"
        "uniform float diffWeight;\n"
        "uniform bool ignoreAlpha;\n"
        "void main() {\n"
        "   vec2 coordL = imageLReframing[1]*srcCoord+imageLReframing[0];\n"
        "   vec2 coordR = imageRReframing[1]*srcCoord+imageRReframing[0];\n"
        "   vec4 colL = (coordL.x<0.0||coordL.y<0.0||coordL.x>1.0||coordL.y>1.0) ? vec4(0.0) : texture2D(srcImageL, coordL);\n"
        "   vec4 colR = (coordR.x<0.0||coordR.y<0.0||coordR.x>1.0||coordR.y>1.0) ? vec4(0.0) : texture2D(srcImageR, coordR);\n";

    switch (type) {
        case ImageComparisonShader::Type::DIFF:
            ss << getDiffFragmentShaderBody();
            break;
        case ImageComparisonShader::Type::SLIDER:
            ss << getSliderFragmentShaderBody();
            break;
        case ImageComparisonShader::Type::FADE:
            ss << getFadeFragmentShaderBody();
            break;
    }

    ss <<
        "}\n";
    return ss.str();
}
}

ImageComparisonShader::ImageComparisonShader() {}

bool ImageComparisonShader::initialize(const RGIShader::SharedVertexShader &sharedVertexShader, Type type) {
    type_ = type;
    return RGIShader::initialize(sharedVertexShader, "diagnostics-image-compare-fs");
}

void ImageComparisonShader::bind(const ScaledBounds &srcBoundsL, const ScaledBounds &srcBoundsR, const ScaledBounds& dstBounds, int srcUnitL, int srcUnitR, float sliderXPos, float sliderStrokeWidth, float diffWeight, bool ignoreAlpha) {
    RGIShader::bind(dstBounds, dstBounds);

    const float imageLReframing[4] = {
        static_cast<float>((dstBounds.a.x-srcBoundsL.a.x)/srcBoundsL.dimensions().x),
        static_cast<float>((dstBounds.a.y-srcBoundsL.a.y)/srcBoundsL.dimensions().y),
        static_cast<float>(dstBounds.dimensions().x/srcBoundsL.dimensions().x),
        static_cast<float>(dstBounds.dimensions().y/srcBoundsL.dimensions().y)
    };
    const float imageRReframing[4] = {
        static_cast<float>((dstBounds.a.x-srcBoundsR.a.x)/srcBoundsR.dimensions().x),
        static_cast<float>((dstBounds.a.y-srcBoundsR.a.y)/srcBoundsR.dimensions().y),
        static_cast<float>(dstBounds.dimensions().x/srcBoundsR.dimensions().x),
        static_cast<float>(dstBounds.dimensions().y/srcBoundsR.dimensions().y)
    };

    unifSrcImageL_.setInt(srcUnitL);
    unifSrcImageR_.setInt(srcUnitR);
    unifSliderXPos_.setFloat(sliderXPos);
    unifSliderStrokeWidth_.setFloat(sliderStrokeWidth);
    unifDiffWeight_.setFloat(diffWeight);
    unifImageLReframing_.setMat2(imageLReframing);
    unifImageRReframing_.setMat2(imageRReframing);
    unifIgnoreAlpha_.setBool(ignoreAlpha);
}

bool ImageComparisonShader::initializeFragmentShader(FragmentShader &fragmentShader) const {
    const std::string fragmentShaderSource = getFragmentShader(type_);

    const GLchar* src[3] = { ODE_GLOBAL_SHADER_PREAMBLE, fragmentShaderSource.c_str() };
    GLint sln[3] = { sizeof(ODE_GLOBAL_SHADER_PREAMBLE)-1, static_cast<GLint>(fragmentShaderSource.size()) };
    int srcCount = 2;

    return fragmentShader.initialize(src, sln, srcCount);
}

void ImageComparisonShader::initializeUniforms() {
    unifSrcImageL_ = shader_.getUniform("srcImageL");
    unifSrcImageR_ = shader_.getUniform("srcImageR");
    unifSliderXPos_ = shader_.getUniform("sliderXPos");
    unifSliderStrokeWidth_ = shader_.getUniform("sliderStrokeWidth");
    unifDiffWeight_ = shader_.getUniform("diffWeight");
    unifImageRReframing_ = shader_.getUniform("imageRReframing");
    unifImageLReframing_ = shader_.getUniform("imageLReframing");
    unifImageRReframing_ = shader_.getUniform("imageRReframing");
    unifIgnoreAlpha_ = shader_.getUniform("ignoreAlpha");
}
