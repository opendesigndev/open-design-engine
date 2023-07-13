
#include "Component.h"

#include <cstring>
#include <octopus/parser.h>
#include "../core/planar-intersections.h"
#include "../core/octopus-type-conversions.h"
#include "../render-assembly/assembly.h"
#include "../render-assembly/graph-transform.h"
#include "../animation/animate.h"
#include "layer-change-apply.h"

namespace ode {

// TODO MOVE?
static void listLayerMissingFonts(std::set<std::string> &names, const octopus::Layer &layer) {
    switch (layer.type) {
        case octopus::Layer::Type::TEXT:
            if (layer.text.has_value()) {
                std::vector<std::string> missingFonts = odtr::listMissingFonts(TEXT_RENDERER_CONTEXT, layer.text.value());
                for (std::string &missingFont : missingFonts)
                    names.insert((std::string &&) missingFont);
            }
            break;
        case octopus::Layer::Type::MASK_GROUP:
            if (layer.mask.has_value())
                listLayerMissingFonts(names, *layer.mask);
            // fallthrough
        case octopus::Layer::Type::GROUP:
            if (layer.layers.has_value()) {
                for (const octopus::Layer &child : layer.layers.value())
                    listLayerMissingFonts(names, child);
            }
            break;
        default:;
    }
}

Result<ComponentPtr, DesignError> Component::create(const octopus::Component &componentManifest, ResourceBase *resourceBase) {
    if (const std::string *octopusString = resourceBase->getOctopus(componentManifest.location)) {
        octopus::Octopus octopus;
        if (octopus::Parser::Error parseError = octopus::Parser::parse(octopus, octopusString->c_str())) {
            return parseError;
        } else {
            ComponentPtr component(new Component);
            if (DesignError error = component->initialize((octopus::Octopus &&) octopus)) {
                return error;
            } else {
                component->id = componentManifest.id;
                component->name = componentManifest.name;
                component->position = Vector2d(componentManifest.bounds.x, componentManifest.bounds.y);
                return (ComponentPtr &&) component;
            }
        }
    }
    return DesignError::OCTOPUS_UNAVAILABLE;
}

Component::Component() { }

Component::Component(const std::string &id) : id(id) { }

DesignError Component::initialize(const octopus::Octopus &octopus) {
    if (this->octopus.content.has_value())
        return DesignError::ALREADY_INITIALIZED;
    this->octopus = octopus;
    return DesignError::OK;
}

DesignError Component::initialize(octopus::Octopus &&octopus) {
    if (this->octopus.content.has_value())
        return DesignError::ALREADY_INITIALIZED;
    id = octopus.id;
    this->octopus = (octopus::Octopus &&) octopus;
    return DesignError::OK;
}

DesignError Component::setFontBase(const FontBasePtr &fontBase) {
    this->fontBase = fontBase;
    return DesignError::OK;
}

static LayerAnimation transformAnimation(const LayerAnimation &animation, const TransformationMatrix &matrix) {
    switch (animation.type) {
        case LayerAnimation::TRANSFORM:
            {
                LayerAnimation transformedAnimation(animation);
                for (LayerAnimation::Keyframe &keyframe : transformedAnimation.keyframes) {
                    if (keyframe.transform.has_value()) {
                        static_assert(sizeof(TransformationMatrix) == 6*sizeof(double), "Assuming TransformationMatrix has POD layout");
                        TransformationMatrix m = inverse(matrix)*TransformationMatrix(keyframe.transform.value().data())*matrix;
                        memcpy(keyframe.transform.value().data(), &m, sizeof(m));
                    }
                }
                return transformedAnimation;
            }
        case LayerAnimation::ROTATION:
            {
                ODE_ASSERT(animation.rotationCenter.has_value());
                LayerAnimation transformedAnimation(animation);
                if (transformedAnimation.rotationCenter.has_value()) {
                    Vector2d center = inverse(matrix)*Vector3d(transformedAnimation.rotationCenter.value()[0], transformedAnimation.rotationCenter.value()[1], 1);
                    transformedAnimation.rotationCenter.value()[0] = center.x;
                    transformedAnimation.rotationCenter.value()[1] = center.y;
                }
                return transformedAnimation;
            }
            break;
        case LayerAnimation::OPACITY:
        case LayerAnimation::FILL_COLOR:
            break;
    }
    return animation;
}

void Component::addSubtreeAnimation(const LayerAnimation &animation, const std::list<octopus::Layer> &layers) {
    for (const octopus::Layer &layer : layers) {
        if (LayerInstance *instance = findInstance(layer.id)) {
            LayerAnimation subAnimation = transformAnimation(animation, TransformationMatrix((*instance)->transform));
            instance->addAnimation(subAnimation);
            if ((*instance)->layers.has_value()) {
                addSubtreeAnimation(subAnimation, (*instance)->layers.value());
            }
        }
    }
}

DesignError Component::setAnimation(const DocumentAnimation &animation) {
    DesignError result = DesignError::OK;
    for (auto &instance : instances)
        instance.second.clearAnimations();
    if (DesignError error = requireBuild())
        return error;
    allAnimations = animation;
    for (const LayerAnimation &layerAnim : animation.animations) {
        if (LayerInstance *instance = findInstance(layerAnim.layer)) {
            instance->addAnimation(layerAnim);
            if ((*instance)->layers.has_value()) {
                addSubtreeAnimation(layerAnim, (*instance)->layers.value());
            }
        } else
            result = DesignError::LAYER_NOT_FOUND;
    }
    ++rev;
    return result;
}

DesignError Component::setPosition(const Vector2d &position) {
    this->position = position;
    return DesignError::OK;
}

DesignError Component::setName(const std::string &name) {
    this->name = name;
    return DesignError::OK;
}

DesignError Component::setRootLayer(const octopus::Layer &layer) {
    return DesignError::NOT_IMPLEMENTED;
}

DesignError Component::addLayer(const std::string &parent, const std::string &before, const octopus::Layer &layer) {
    if (DesignError error = requireBuild())
        return error;
    if (findInstance(layer.id))
        return DesignError::DUPLICATE_LAYER_ID;
    if (LayerInstance *parentInstance = findInstance(parent)) {
        if (*parentInstance) {
            if (!((*parentInstance)->type == octopus::Layer::Type::GROUP || (*parentInstance)->type == octopus::Layer::Type::MASK_GROUP))
                return DesignError::WRONG_LAYER_TYPE;
            ODE_ASSERT((*parentInstance)->layers.has_value());
            if (before.empty())
                (*parentInstance)->layers->push_back(layer);
            else {
                std::list<octopus::Layer>::iterator it = (*parentInstance)->layers->begin();
                for (; it != (*parentInstance)->layers->end(); ++it) {
                    if (it->id == before) {
                        (*parentInstance)->layers->insert(it, layer);
                        break;
                    }
                }
                if (it == (*parentInstance)->layers->end())
                    return DesignError::LAYER_NOT_FOUND;
            }
            ++rev;
            buildComplete = false;
            return DesignError::OK;
        }
    }
    return DesignError::LAYER_NOT_FOUND;
}

DesignError Component::removeLayer(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id)) {
        const std::string &parentId = instance->getParentId();
        if (LayerInstance *parentInstance = findInstance(parentId)) {
            nonstd::optional<std::list<octopus::Layer>> &layers = (*parentInstance)->layers;
            ODE_ASSERT(layers.has_value());
            const std::list<octopus::Layer>::iterator layerInParentIt = std::find_if(layers->begin(), layers->end(), [&id](const octopus::Layer &layer) {
                return layer.id == id;
            });
            if (layerInParentIt != layers->end()) {
                layers->erase(layerInParentIt);
                std::vector<std::string> instancesToErase;
                for (const std::pair<const std::string, ode::LayerInstance> &instance : instances) {
                    if (isInstanceInSubtree(id, instance.first)) {
                        instancesToErase.emplace_back(instance.first);
                    }
                }
                for (const std::string &instanceToErase : instancesToErase) {
                    instances.erase(instanceToErase);
                }
                ++rev;
                buildComplete = false;
                return DesignError::OK;
            }
        }
    }
    return DesignError::LAYER_NOT_FOUND;
}

DesignError Component::modifyLayer(const std::string &id, const octopus::LayerChange &layerChange) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id)) {
        octopus::Layer &layer = **instance;
        if (Result<ChangeLevel, DesignError> result = applyLayerChange(layer, layerChange)) {
            switch (result.value()) {
                case ChangeLevel::HIERARCHY:
                    buildComplete = false;
                    // fallthrough
                case ChangeLevel::COMPOSITION:
                    ++rev;
                    // fallthrough
                case ChangeLevel::BOUNDS:
                case ChangeLevel::VISUAL:
                case ChangeLevel::LOGICAL:
                case ChangeLevel::NONE:;
            }
            // TODO: Force re-initialize instance shape and text
            instance->invalidate();
            if (layer.type == octopus::Layer::Type::SHAPE) {
                instance->initializeShape();
            }
            if (layer.type == octopus::Layer::Type::TEXT) {
                instance->initializeText(fontBase.get());
            }
            return DesignError::OK;
        } else
            return result.error();
    }
    return DesignError::LAYER_NOT_FOUND;
}

DesignError Component::transformLayer(const std::string &id, octopus::Fill::Positioning::Origin basis, const TransformationMatrix &transformation) {
    if (!transformation.invertible())
        return DesignError::SINGULAR_TRANSFORMATION;
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id)) {
        octopus::Layer &layer = *(octopus::Layer *) *instance;
        TransformationMatrix layerTranformation = fromOctopusTransform(layer.transform);
        switch (basis) {
            case octopus::Fill::Positioning::Origin::LAYER:
                layerTranformation = layerTranformation*transformation;
                break;
            case octopus::Fill::Positioning::Origin::PARENT:
                layerTranformation = transformation*layerTranformation;
                break;
            case octopus::Fill::Positioning::Origin::ARTBOARD:
                ODE_ASSERT(!"TODO");
                // fallthrough
            case octopus::Fill::Positioning::Origin::COMPONENT:
                {
                    TransformationMatrix ancestorTransform = ((LayerInstanceSpecifier) *instance).parentTransform;
                    if (!ancestorTransform.invertible())
                        return DesignError::SINGULAR_TRANSFORMATION;
                    layerTranformation = inverse(ancestorTransform)*transformation*ancestorTransform*layerTranformation;
                }
                break;
        }
        toOctopusTransform(layer.transform, layerTranformation);
        instance->invalidateBounds();
        buildComplete = false;
        return DesignError::OK;
    }
    return DesignError::LAYER_NOT_FOUND;
}

DesignError Component::notifyReference(const std::string &referencelayerId, const std::string &masterLayerId, const octopus::LayerChange *layerChange) {
    return DesignError::NOT_IMPLEMENTED;
}

const octopus::Octopus &Component::getOctopus() const {
    return octopus;
}

const Vector2d &Component::getPosition() const {
    return position;
}

Result<octopus::Octopus, DesignError> Component::buildOctopus() {
    return DesignError::NOT_IMPLEMENTED;
}

Result<const octopus::Layer *, DesignError> Component::getLayerOctopus(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return (const octopus::Layer *) instance;
    else
        return DesignError::LAYER_NOT_FOUND;
}

Result<octopus::Octopus, DesignError> Component::buildLayerOctopus(const std::string &id, bool recursive) {
    return DesignError::NOT_IMPLEMENTED;
}

Result<LayerBounds, DesignError> Component::getLayerBounds(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return instance->bounds();
    else
        return DesignError::LAYER_NOT_FOUND;
}

Result<LayerMetrics, DesignError> Component::getLayerMetrics(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id)) {
        ODE_ASSERT(*instance);
        return instance->metrics();
    } else
        return DesignError::LAYER_NOT_FOUND;
}

Result<Rasterizer::Shape *, DesignError> Component::getLayerShape(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return instance->shape();
    else
        return DesignError::LAYER_NOT_FOUND;
}

Result<TextShapeHolder *, DesignError> Component::getLayerTextShape(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return &instance->textShape();
    else
        return DesignError::LAYER_NOT_FOUND;
}

Result<const DocumentAnimation *, DesignError> Component::getAnimation(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return &instance->animation();
    else
        return DesignError::LAYER_NOT_FOUND;
}

Result<LayerAnimation::Keyframe, DesignError> Component::getAnimationValue(int index, double time) const {
    if (!(index >= 0 && index < (int) allAnimations.animations.size()))
        return DesignError::ITEM_NOT_FOUND;
    return animate(allAnimations.animations[index], time);
}

std::string Component::identifyLayer(const Vector2d &position, double radius) {
    ODE_ASSERT(radius >= 0);
    if (octopus.content.has_value()) {
        if (requireBuild())
            return std::string();
        if (LayerInstance *instance = findInstance(octopus.content->id)) {
            std::string id;
            if (identifyLayer(id, *instance, position, radius))
                return id;
        }
    }
    return std::string();
}

void Component::listMissingFonts(std::set<std::string> &names) const {
    if (octopus.content.has_value())
        listLayerMissingFonts(names, *octopus.content);
}

Result<Rendexptr, DesignError> Component::assemble() {
    if (DesignError error = requireBuild())
        return error;
    if (!octopus.content.has_value())
        return DesignError::LAYER_NOT_FOUND;
    if (octopus.content->visible) {
        if (Result<Rendexptr, DesignError> result = assembleLayer(octopus.content->id))
            return resolveBackground(result.value());
        else
            return result.error();
    }
    return Rendexptr();
}

Result<Rendexptr, DesignError> Component::assembleLayer(const std::string &id) {
    if (DesignError error = requireBuild())
        return error;
    if (LayerInstance *instance = findInstance(id))
        return assembleLayer(*instance, nonstd::optional<octopus::MaskBasis>()).root;
    else
        return DesignError::LAYER_NOT_FOUND;
}

DesignError Component::requireBuild() {
    if (buildComplete)
        return DesignError::OK;
    return rebuild();
}

DesignError Component::rebuild() {
    if (octopus.content.has_value()) {
        Result<LayerInstance *, DesignError> result = rebuildSubtree(octopus.content.get(), TransformationMatrix::identity, 1, std::string());
        if (result.failure())
            return result.error();
    }
    buildComplete = true;
    return DesignError::OK;
}

Result<LayerInstance *, DesignError> Component::rebuildSubtree(octopus::Layer *layer, const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId) {
    LayerInstance &instance = instances[layer->id]; // TODO full instance id
    if ((const octopus::Layer *) instance != layer) {
        instance = LayerInstance(layer, parentTransform, parentFeatureScale, parentId);
    } else
        instance.setParent(parentTransform, parentFeatureScale, parentId);
    bool masked = false;
    switch (instance->type) {
        case octopus::Layer::Type::SHAPE:
            if (!instance.initializeShape())
                return DesignError::SHAPE_LAYER_ERROR;
            break;
        case octopus::Layer::Type::TEXT:
            if (!instance.initializeText(fontBase.get()))
                return DesignError::TEXT_LAYER_ERROR;
            break;
        case octopus::Layer::Type::GROUP:
        case octopus::Layer::Type::MASK_GROUP:
            {
                bool masked = false;
                TransformationMatrix transform = instance.transformation();
                double featureScale = instance.featureScale();
                UntransformedBounds contentBounds = UntransformedBounds::unspecified;
                if (instance->type == octopus::Layer::Type::MASK_GROUP && instance->mask.has_value()) {
                    masked = instance->maskBasis.value_or(octopus::MaskBasis::SOLID) != octopus::MaskBasis::SOLID;
                    Result<LayerInstance *, DesignError> subtree = rebuildSubtree(instance->mask.get(), transform, featureScale, instance.id());
                    if (subtree.failure())
                        return subtree.error();
                    contentBounds |= subtree.value()->bounds().logicalBounds;
                }
                if (instance->layers.has_value()) {
                    for (octopus::Layer &child : instance->layers.value()) {
                        Result<LayerInstance *, DesignError> subtree = rebuildSubtree(&child, transform, featureScale, instance.id());
                        if (subtree.failure())
                            return subtree.error();
                        if (!masked)
                            contentBounds |= subtree.value()->bounds().logicalBounds;
                    }
                }
                instance.setLogicalBounds(contentBounds);
            }
            break;
        case octopus::Layer::Type::COMPONENT_REFERENCE:
        case octopus::Layer::Type::COMPONENT_INSTANCE:
            ODE_ASSERT(!"Not implemented");
            return DesignError::NOT_IMPLEMENTED;
    }
    return &instance;
}

int Component::assemblyFlags(const LayerInstance &instance) {
    bool containsOpacityAnimation = false;
    for (const LayerAnimation &animation : instance.animation().animations) {
        if (animation.type == LayerAnimation::OPACITY) {
            containsOpacityAnimation = true;
            break;
        }
    }
    return containsOpacityAnimation ? 0 : ASSEMBLY_FLAG_FIXED_OPACITY;
}

LayerInstance *Component::findInstance(const std::string &id) {
    ODE_ASSERT(buildComplete);
    std::map<std::string, LayerInstance>::iterator it = instances.find(id);
    if (it != instances.end())
        return &it->second;
    return nullptr;
}

bool Component::isInstanceInSubtree(const std::string &subtreeId, const std::string &instanceId) {
    if (LayerInstance *instance = findInstance(instanceId)) {
        if (instanceId == subtreeId) {
            return true;
        }
        return isInstanceInSubtree(subtreeId, instance->getParentId());
    }
    return false;
}

RendexSubtree Component::assembleLayer(const LayerInstance &instance, const nonstd::optional<octopus::MaskBasis> &maskBasis) {
    int flags = assemblyFlags(instance);
    switch (instance->type) {
        case octopus::Layer::Type::SHAPE:
            return assembleShapeLayer(instance, instance.bounds(), maskBasis, flags);
        case octopus::Layer::Type::TEXT:
            return assembleTextLayer(instance, instance.bounds(), maskBasis, flags);
        case octopus::Layer::Type::GROUP:
        case octopus::Layer::Type::MASK_GROUP:
            if (instance->layers.has_value()) {
                LayerInstanceSpecifier layer(instance);
                GroupLayerAssembler groupAssembler(layer, instance.bounds(), maskBasis, flags);
                if (instance->type == octopus::Layer::Type::MASK_GROUP && instance->mask.has_value()) {
                    if (LayerInstance *maskInstance = findInstance(instance->mask->id))
                        groupAssembler.setMask(*maskInstance, assembleLayer(*maskInstance, instance->maskBasis));
                }
                for (const octopus::Layer &child : instance->layers.value()) {
                    if (child.visible) {
                        if (LayerInstance *childInstance = findInstance(child.id))
                            groupAssembler.addLayer(assembleLayer(*childInstance, nonstd::optional<octopus::MaskBasis>()));
                    }
                }
                return groupAssembler.get();
            }
            break;
        case octopus::Layer::Type::COMPONENT_REFERENCE:
            ODE_ASSERT(!"NOT IMPLEMENTED");
            break;
        case octopus::Layer::Type::COMPONENT_INSTANCE:
            ODE_ASSERT(!"NOT IMPLEMENTED");
            break;
    }
    return RendexSubtree();
}

bool Component::identifyLayer(std::string &id, const LayerInstance &instance, const Vector2d &position, double radius) {
    ODE_ASSERT(instance);
    std::string maskId;
    bool maskHit = false;
    if (instance->type == octopus::Layer::Type::MASK_GROUP && instance->mask.has_value()) {
        if (LayerInstance *maskInstance = findInstance(instance->mask->id))
            maskHit = identifyLayer(maskId, *maskInstance, position, radius);
        // Filter children by mask
        if (!(maskHit || instance->maskBasis.value_or(octopus::MaskBasis::BODY) == octopus::MaskBasis::SOLID))
            return false;
        if (!instance->mask->visible)
            maskHit = false;
    }
    if (instance->layers.has_value()) {
        for (std::list<octopus::Layer>::const_reverse_iterator it = instance->layers->rbegin(); it != instance->layers->rend(); ++it) {
            if (it->visible) {
                if (LayerInstance *childInstance = findInstance(it->id)) {
                    if (identifyLayer(id, *childInstance, position, radius))
                        return true;
                }
            }
        }
    }
    if (instance->type == octopus::Layer::Type::SHAPE || instance->type == octopus::Layer::Type::TEXT) {
        const UntransformedBounds &bounds = instance.bounds().untransformedBounds;
        if (bounds && intersects(bounds, instance.transformation(), position, radius)) {
            id = instance->id;
            return true;
        }
    }
    if (maskHit) {
        id = (std::string &&) maskId;
        return true;
    }
    return false;
}

}
