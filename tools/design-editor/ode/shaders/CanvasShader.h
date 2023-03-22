
#pragma once

#include "DesignEditorShader.h"

using namespace ode;

/// Selection rectangle canvas annotation. In canvas space <0,1>x<0,1>.
using SelectionRectangleOpt = std::optional<ode::Rectangle<float>>;

/// A Design Editor shader that draws the Octopus rendering output and annotations.
class CanvasShader : public DesignEditorShader {
public:
    CanvasShader();
    CanvasShader(const CanvasShader&) = delete;
    CanvasShader& operator=(const CanvasShader&) = delete;
    virtual ~CanvasShader() = default;

    bool initialize(const DesignEditorShader::SharedVertexShader &sharedVertexShader);
    void bind(const ScaledBounds& srcBounds,
              const ScaledBounds& dstBounds,
              int srcUnit,
              const Vector2i &resolution,
              const SelectionRectangleOpt &selectionRectangle,
              bool ignoreAlpha);

protected:
    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const override;
    virtual void initializeUniforms() override;

    Uniform unifSrcImage_;
    Uniform unifResolution_;
    Uniform unifIgnoreAlpha_;
    Uniform unifSelectionRectangle_;
};
