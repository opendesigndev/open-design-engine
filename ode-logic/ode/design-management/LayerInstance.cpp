
#include "LayerInstance.h"

namespace ode {

LayerInstance::LayerInstance(octopus::Layer *layer, const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId) : layer(layer), parentTransform(parentTransform), parentFeatureScale(parentFeatureScale), parentId(parentId) { }

bool LayerInstance::initializeShape() {
    if (statusFlags&FLAG_SHAPE_UP_TO_DATE)
        return true;
    ODE_ASSERT(layer && layer->type == octopus::Layer::Type::SHAPE);
    if (layer->shape.has_value()) {
        TransformationMatrix transform = transformation();
        if (!(shape = Rasterizer::createShape(layer->shape.value())))
            return false;
        layerBounds.logicalBounds = (UntransformedBounds) Rasterizer::getBounds(shape.get(), Rasterizer::BODY, Matrix3x2d(1));
        layerBounds.untransformedBounds = layerBounds.logicalBounds;
        layerBounds.bounds = (UnscaledBounds) Rasterizer::getBounds(shape.get(), Rasterizer::BODY, transform);
        for (size_t i = 0; i < layer->shape->strokes.size(); ++i) {
            if (layer->shape->strokes[i].position != octopus::Stroke::Position::INSIDE) {
                layerBounds.untransformedBounds |= (UntransformedBounds) Rasterizer::getBounds(shape.get(), int(i), Matrix3x2d(1));
                layerBounds.bounds |= (UnscaledBounds) Rasterizer::getBounds(shape.get(), int(i), transform);
            }
        }
        statusFlags |= FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE;
        return true;
    }
    return false;
}

bool LayerInstance::initializeText(FontBase *fontBase) {
    if (statusFlags&FLAG_SHAPE_UP_TO_DATE)
        return true;
    // TODO layer transformation & bounds
    if (layer->text.has_value()) {
        if (fontBase)
            fontBase->loadFonts(layer->text.value());

        if ((textShape = odtr::shapeText(TEXT_RENDERER_CONTEXT, layer->text.value()))) {
            layerBounds.bounds = fromTextRendererBounds(odtr::getBounds(TEXT_RENDERER_CONTEXT, textShape));
            layerBounds.logicalBounds = (UntransformedBounds) layerBounds.bounds;
            layerBounds.untransformedBounds = layerBounds.logicalBounds;
        }

        statusFlags |= FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE;
        return true;
    }
    return false;
}

void LayerInstance::setLogicalBounds(const UntransformedBounds &bounds) {
    layerBounds.logicalBounds = bounds.canonical();
}

void LayerInstance::setParent(const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId) {
    if (this->parentTransform != parentTransform) {
        // TODO
        this->parentTransform = parentTransform;
    }
    if (this->parentFeatureScale != parentFeatureScale) {
        // TODO
        this->parentFeatureScale = parentFeatureScale;
    }
    this->parentId = parentId;
}

void LayerInstance::invalidate() {
    statusFlags &= ~(FLAG_TRANSFORM_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE|FLAG_SHAPE_UP_TO_DATE);
}

void LayerInstance::clearAnimations() {
    anim.animations.clear();
}

void LayerInstance::addAnimation(const LayerAnimation &animation) {
    anim.animations.push_back(animation);
}

std::string LayerInstance::id() const {
    ODE_ASSERT(layer);
    // TODO disambiguate instances of the same layer
    return layer->id;
}

const DocumentAnimation &LayerInstance::animation() const {
    return anim;
}

const LayerBounds &LayerInstance::bounds() const {
    return layerBounds;
}

LayerMetrics LayerInstance::metrics() const {
    LayerMetrics metrics;
    static_cast<LayerBounds &>(metrics) = layerBounds;
    metrics.transformation = transformation();
    metrics.featureScale = featureScale();
    return metrics;
}

TransformationMatrix LayerInstance::transformation() const {
    ODE_ASSERT(layer);
    return parentTransform*TransformationMatrix(layer->transform);
}

double LayerInstance::featureScale() const {
    ODE_ASSERT(layer);
    return parentFeatureScale*layer->featureScale.value_or(1);
}

Rasterizer::Shape *LayerInstance::getShape() {
    return shape.get();
}

odtr::TextShapeHandle LayerInstance::getTextShape() {
    return textShape;
}

const std::string &LayerInstance::getParentId() const {
    return parentId;
}

octopus::Layer *LayerInstance::operator->() {
    return layer;
}

const octopus::Layer *LayerInstance::operator->() const {
    return layer;
}

LayerInstance::operator octopus::Layer *() {
    return layer;
}

LayerInstance::operator const octopus::Layer *() const {
    return layer;
}

LayerInstance::operator LayerInstanceSpecifier() const {
    return LayerInstanceSpecifier {
        layer,
        parentTransform,
        parentFeatureScale
    };
}

}
