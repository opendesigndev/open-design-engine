
#include "LayerInstance.h"

namespace ode {

// TODO: Duplicated from Renderer.cpp - unify or remove
static UnscaledBounds transformBounds(const UntransformedBounds &bounds, const TransformationMatrix &matrix) {
    Vector2d a = matrix*Vector3d(bounds.a.x, bounds.a.y, 1);
    Vector2d b = matrix*Vector3d(bounds.b.x, bounds.a.y, 1);
    Vector2d c = matrix*Vector3d(bounds.a.x, bounds.b.y, 1);
    Vector2d d = matrix*Vector3d(bounds.b.x, bounds.b.y, 1);
    return UnscaledBounds(
        std::min(std::min(std::min(a.x, b.x), c.x), d.x),
        std::min(std::min(std::min(a.y, b.y), c.y), d.y),
        std::max(std::max(std::max(a.x, b.x), c.x), d.x),
        std::max(std::max(std::max(a.y, b.y), c.y), d.y)
    );
}

LayerInstance::LayerInstance(octopus::Layer *layer, const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId) : layer(layer), parentTransform(parentTransform), parentFeatureScale(parentFeatureScale), parentId(parentId) { }

bool LayerInstance::initializeShape() {
    if ((statusFlags&(FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE)) == (FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE))
        return true;
    ODE_ASSERT(layer && layer->type == octopus::Layer::Type::SHAPE);
    if (layer->shape.has_value()) {
        TransformationMatrix transform = transformation();
        if (!((statusFlags&FLAG_SHAPE_UP_TO_DATE) || (shape = Rasterizer::createShape(layer->shape.value()))))
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
    if ((statusFlags&(FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE)) == (FLAG_SHAPE_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE))
        return true;
    ODE_ASSERT(layer && layer->type == octopus::Layer::Type::TEXT);
    // TODO layer transformation & bounds
    if (layer->text.has_value()) {
        if (!(statusFlags&FLAG_SHAPE_UP_TO_DATE)) {
            if (fontBase)
                fontBase->loadFonts(layer->text.value());
            textShape = odtr::shapeText(TEXT_RENDERER_CONTEXT, layer->text.value());
        }
        if (textShape) {
            layerBounds.logicalBounds = (UntransformedBounds) fromTextRendererBounds(odtr::getBounds(TEXT_RENDERER_CONTEXT, textShape));
            layerBounds.untransformedBounds = layerBounds.logicalBounds;
            layerBounds.bounds = transformBounds(layerBounds.logicalBounds, transformation());
        } else
            layerBounds = LayerBounds();
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
        statusFlags &= ~FLAG_BOUNDS_UP_TO_DATE;
        this->parentTransform = parentTransform;
    }
    if (this->parentFeatureScale != parentFeatureScale) {
        statusFlags &= ~FLAG_BOUNDS_UP_TO_DATE;
        this->parentFeatureScale = parentFeatureScale;
    }
    this->parentId = parentId;
}

void LayerInstance::invalidate() {
    statusFlags &= ~(FLAG_TRANSFORM_UP_TO_DATE|FLAG_BOUNDS_UP_TO_DATE|FLAG_SHAPE_UP_TO_DATE);
}

void LayerInstance::invalidateBounds() {
    statusFlags &= ~FLAG_BOUNDS_UP_TO_DATE;
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
