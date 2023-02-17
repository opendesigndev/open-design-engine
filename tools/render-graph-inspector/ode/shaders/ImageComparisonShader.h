
#pragma once

#include "RGIShader.h"

using namespace ode;

class ImageComparisonShader : public RGIShader {
public:
    enum class Type : uint8_t {
        DIFF = 0,
        SLIDER = 1,
        FADE = 2,
    };

    ImageComparisonShader();
    ImageComparisonShader(const ImageComparisonShader&) = delete;
    ImageComparisonShader& operator=(ImageComparisonShader&) = delete;
    virtual ~ImageComparisonShader() = default;

    bool initialize(const RGIShader::SharedVertexShader &sharedVertexShader, Type type);
    void bind(const ScaledBounds &srcBoundsL, const ScaledBounds &srcBoundsR, const ScaledBounds &dstBounds, int srcUnitL, int srcUnitR, float sliderXPos, float sliderStrokeWidth, float diffWeight, bool ignoreAlpha);

protected:
    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const override;
    virtual void initializeUniforms() override;

    Type type_;
    Uniform unifSrcImageL_;
    Uniform unifSrcImageR_;
    Uniform unifSliderXPos_;
    Uniform unifSliderStrokeWidth_;
    Uniform unifDiffWeight_;
    Uniform unifImageLReframing_;
    Uniform unifImageRReframing_;
    Uniform unifIgnoreAlpha_;
};
