
#pragma once

#include "DesignEditorShader.h"

using namespace ode;

class SimpleBlitShader : public DesignEditorShader {
public:
    SimpleBlitShader();
    SimpleBlitShader(const SimpleBlitShader&) = delete;
    SimpleBlitShader& operator=(const SimpleBlitShader&) = delete;
    virtual ~SimpleBlitShader() = default;

    bool initialize(const DesignEditorShader::SharedVertexShader &sharedVertexShader);
    void bind(const ScaledBounds& srcBounds, const ScaledBounds& dstBounds, int srcUnit, bool ignoreAlpha);

protected:
    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const override;
    virtual void initializeUniforms() override;

    Uniform unifSrcImage_;
    Uniform unifIgnoreAlpha_;
};
