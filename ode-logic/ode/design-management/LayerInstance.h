
#pragma once

#include <string>
#include <octopus/octopus.h>
#include <ode-essentials.h>
#include <ode-rasterizer.h>
#include "../core/TransformationMatrix.h"
#include "../core/LayerBounds.h"
#include "../core/LayerMetrics.h"
#include "../core/LayerInstanceSpecifier.h"
#include "../text-renderer/text-renderer.h"
#include "../animation/DocumentAnimation.h"

namespace ode {

/// The internal state of a layer instance
class LayerInstance {

public:
    static constexpr int FLAG_TRANSFORM_UP_TO_DATE = 0x01;
    static constexpr int FLAG_BOUNDS_UP_TO_DATE = 0x02;
    static constexpr int FLAG_SHAPE_UP_TO_DATE = 0x04;
    static constexpr int FLAG_SMALL_SHAPE = 0x1000; // is guaranteed not to be a big shape
    static constexpr int FLAG_BIG_SHAPE = 0x2000; // Skia preprocessing takes too long and should be run in parallel

    LayerInstance() = default;
    LayerInstance(octopus::Layer *layer, const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId);

    bool initializeShape();
    bool initializeText(FontBase *fontBase);
    void setLogicalBounds(const UntransformedBounds &bounds);
    void setParent(const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId);
    void invalidate();
    void clearAnimations();
    void addAnimation(const LayerAnimation &animation);

    std::string id() const;
    const DocumentAnimation &animation() const;
    const LayerBounds &bounds() const;
    LayerMetrics metrics() const;
    TransformationMatrix transformation() const;
    double featureScale() const;
    Rasterizer::Shape *getShape();
    odtr::TextShapeHandle getTextShape();

    octopus::Layer *operator->();
    const octopus::Layer *operator->() const;
    operator octopus::Layer *();
    operator const octopus::Layer *() const;
    operator LayerInstanceSpecifier() const;

private:
    octopus::Layer *layer = nullptr;
    TransformationMatrix parentTransform;
    double parentFeatureScale = 1;
    std::string parentId;

    int statusFlags = 0;
    LayerBounds layerBounds;

    Rasterizer::ShapePtr shape;
    TextShapeHolder textShape;

    DocumentAnimation anim;

};

}
