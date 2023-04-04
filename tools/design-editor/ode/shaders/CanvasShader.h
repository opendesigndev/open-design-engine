
#pragma once

#include "DesignEditorShader.h"

using namespace ode;

/// Rectangle canvas annotation. In canvas space <0,1>x<0,1>.
using AnnotationRectangle = ode::Rectangle<float>;
using AnnotationRectangleOpt = std::optional<AnnotationRectangle>;
using AnnotationRectangles = std::vector<AnnotationRectangle>;

/// A Design Editor shader that draws the Octopus rendering output and annotations.
class CanvasShader : public DesignEditorShader {
private:
    static const size_t MAX_HIGHLIGHT_RECTANGLES = 16;

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
              const AnnotationRectangles &highlightRectangles,
              bool ignoreAlpha);

protected:
    virtual bool initializeFragmentShader(FragmentShader &fragmentShader) const override;
    virtual void initializeUniforms() override;

    Uniform unifSrcImage_;
    Uniform unifResolution_;
    Uniform unifIgnoreAlpha_;
    Uniform unifSelectionRectangle_;
    Uniform unifHighlightRectangles_[MAX_HIGHLIGHT_RECTANGLES];
};
