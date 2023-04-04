
#pragma once

#include "DesignEditorShader.h"

using namespace ode;

/// Rectangle canvas annotation. In canvas space <0,1>x<0,1>.
using AnnotationRectangleOpt = std::optional<ode::Rectangle<float>>;

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
              const AnnotationRectangleOpt &selectionRectangle,
              const AnnotationRectangleOpt &highlightRectangle,
              bool ignoreAlpha);

protected:
    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const override;
    virtual void initializeUniforms() override;

    Uniform unifSrcImage_;
    Uniform unifResolution_;
    Uniform unifIgnoreAlpha_;
    Uniform unifSelectionRectangle_;
    Uniform unifHighlightRectangle_;
};
