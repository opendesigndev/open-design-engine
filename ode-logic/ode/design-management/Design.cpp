
#include "Design.h"

#include <octopus/octopus.h>
#include <octopus/parser.h>

namespace ode {

DesignError Design::ComponentAccessor::setRootLayer(const octopus::Layer &layer) {
    DesignError error = component->setRootLayer(layer);
    if (!error)
        notifyReferences(component, std::string(), nullptr);
    return error;
}

DesignError Design::ComponentAccessor::addLayer(const std::string &parent, const std::string &before, const octopus::Layer &layer) {
    DesignError error = component->addLayer(parent, before, layer);
    if (!error)
        notifyReferences(component, layer.id, nullptr);
    return error;
}

DesignError Design::ComponentAccessor::removeLayer(const std::string &id) {
    DesignError error = component->removeLayer(id);
    if (!error)
        notifyReferences(component, id, nullptr);
    return error;
}

DesignError Design::ComponentAccessor::modifyLayer(const std::string &id, const octopus::LayerChange &layerChange) {
    DesignError error = component->modifyLayer(id, layerChange);
    if (!error)
        notifyReferences(component, id, &layerChange);
    return error;
}

DesignError Design::ComponentAccessor::transformLayer(const std::string &id, octopus::Fill::Positioning::Origin basis, const TransformationMatrix &transformation) {
    DesignError error = component->transformLayer(id, basis, transformation);
    if (!error) {
        octopus::LayerChange layerChange;
        layerChange.subject = octopus::LayerChange::Subject::LAYER;
        layerChange.op = octopus::LayerChange::Op::PROPERTY_CHANGE;
        //layerChange.values.transform = ... if needed
        notifyReferences(component, id, &layerChange);
    }
    return error;
}

DesignError Design::ComponentAccessor::setAnimation(const DocumentAnimation &animation) {
    return component->setAnimation(animation);
}

const octopus::Octopus &Design::ComponentAccessor::getOctopus() const {
    return component->getOctopus();
}

Result<const octopus::Layer *, DesignError> Design::ComponentAccessor::getLayerOctopus(const std::string &id) {
    return component->getLayerOctopus(id);
}

const std::string &Design::ComponentAccessor::getId() const {
    return component->getId();
}

int Design::ComponentAccessor::revision() const {
    return component->revision();
}

Result<octopus::Octopus, DesignError> Design::ComponentAccessor::buildOctopus() const {
    return component->buildOctopus();
}

Result<octopus::Octopus, DesignError> Design::ComponentAccessor::buildLayerOctopus(const std::string &id, bool recursive) const {
    return component->buildLayerOctopus(id, recursive);
}

Result<LayerBounds, DesignError> Design::ComponentAccessor::getLayerBounds(const std::string &id) const {
    return component->getLayerBounds(id);
}

Result<LayerMetrics, DesignError> Design::ComponentAccessor::getLayerMetrics(const std::string &id) {
    return component->getLayerMetrics(id);
}

Result<LayerAnimation::Keyframe, DesignError> Design::ComponentAccessor::getAnimationValue(int index, double time) const {
    return component->getAnimationValue(index, time);
}

std::string Design::ComponentAccessor::identifyLayer(const Vector2d &position, double radius) {
    return component->identifyLayer(position, radius);
}

void Design::ComponentAccessor::listMissingFonts(std::set<std::string> &names) const {
    return component->listMissingFonts(names);
}

Result<Rendexptr, DesignError> Design::ComponentAccessor::assemble() {
    return component->assemble();
}

Design::ComponentAccessor::operator bool() const {
    return design && component;
}

void Design::ComponentAccessor::notifyReferences(Component *component, const std::string &masterLayerId, const octopus::LayerChange *layerChange) {
    for (const ComponentLayerId &ref : design->referenceMap[component->getId()]) {
        auto it = design->components.find(ref.component);
        if (it != design->components.end()) {
            it->second->notifyReference(ref.layer, masterLayerId, layerChange);
            notifyReferences(it->second.get(), ref.layer, nullptr);
        }
    }
}

Result<Design, DesignError> Design::loadManifest(const octopus::OctopusManifest &manifest) {
    Design design;
    design.name = manifest.name;
    for (const octopus::Component &component : manifest.components) {
        if (DesignError error = design.addComponent(component))
            return error;
    }
    // TODO pages
    return (Design &&) design;
}

DesignError Design::addComponent(const octopus::Component &component) {
    if (components.find(component.id) != components.end() || protocomponents.find(component.id) != protocomponents.end())
        return DesignError::DUPLICATE_COMPONENT_ID;
    protocomponents.insert(std::make_pair(component.id, component));
    return DesignError::OK;
}

DesignError Design::addComponent(const octopus::Component &component, octopus::Octopus &&octopus) {
    if (components.find(component.id) != components.end() || protocomponents.find(component.id) != protocomponents.end())
        return DesignError::DUPLICATE_COMPONENT_ID;
    if (octopus.id.empty())
        octopus.id = component.id;
    if (octopus.id != component.id)
        return DesignError::INVALID_COMPONENT;
    ComponentPtr componentPtr(new Component(component.id));
    if (DesignError error = componentPtr->initialize((octopus::Octopus &&) octopus)) {
        return error;
    } else {
        componentPtr->setName(component.name);
        componentPtr->setPosition(Vector2d(component.bounds.x, component.bounds.y));
        onComponentLoaded(componentPtr.get());
        components.insert(std::make_pair(component.id, (ComponentPtr &&) componentPtr));
        return DesignError::OK;
    }
}

DesignError Design::removeComponent(const ComponentAccessor &component) {
    return removeComponent(component.getId());
}

DesignError Design::removeComponent(const std::string &id) {
    auto it = referenceMap.find(id);
    if (it != referenceMap.end()) {
        if (!it->second.empty()) {
            // Note: Wrong error reported if component is referenced but not added (make sure it can't happen?)
            return DesignError::COMPONENT_IN_USE;
        }
    }
    auto c = components.find(id);
    if (c == components.end()) {
        auto p = protocomponents.find(id);
        if (p == protocomponents.end()) {
            return DesignError::COMPONENT_NOT_FOUND;
        } else {
            protocomponents.erase(p);
        }
    } else {
        const octopus::Octopus &octopus = c->second->getOctopus();
        if (octopus.content.has_value())
            recurseUnmapReferences(c->second->getId(), *octopus.content);
        components.erase(c);
    }
    // TODO remove from page
    return DesignError::OK;
}

Design::ComponentAccessor Design::getComponent(const std::string &id) {
    return ComponentAccessor(this, requireComponent(id));
}

void Design::listComponents(std::set<std::string> &ids) const {
    for (const std::map<std::string, ComponentPtr>::value_type &entry : components)
        ids.insert(entry.first);
    for (const std::map<std::string, octopus::Component>::value_type &entry : protocomponents)
        ids.insert(entry.first);
}

void Design::listMissingFonts(std::set<std::string> &names) const {
    for (const std::map<std::string, ComponentPtr>::value_type &entry : components) {
        if (entry.second)
            entry.second->listMissingFonts(names);
    }
    // TODO what about protocomponents?
}

Component *Design::requireComponent(const std::string &id) {
    auto it = components.find(id);
    if (it == components.end()) {
        auto protoIt = protocomponents.find(id);
        if (protoIt != protocomponents.end()) {
            const octopus::Component &componentManifest = protoIt->second;
            if (Result<ComponentPtr, DesignError> result = Component::create(componentManifest, resourceBase)) {
                Component *componentPtr = result.value().get();
                onComponentLoaded(componentPtr);
                components.insert(std::make_pair(id, (ComponentPtr &&) result.value()));
                protocomponents.erase(protoIt);
                return componentPtr;
            } // else report error from result
        } // else report error - unknown ID
        return nullptr;
    }
    return it->second.get();
}

void Design::onComponentLoaded(const Component *component) {
    const octopus::Octopus &octopus = component->getOctopus();
    if (octopus.content.has_value())
        recurseMapReferences(component->getId(), *component->getOctopus().content);
}

void Design::recurseMapReferences(const std::string &componentId, const octopus::Layer &layer) {
    if (layer.type == octopus::Layer::Type::COMPONENT_REFERENCE /* && layer.componentId.has_value() */) {
        // TODO referenceMap[layer.componentId.value()].insert(ComponentLayerId(componentId, layer.id));
    }
    if (layer.layers.has_value()) {
        for (const octopus::Layer &subLayer : layer.layers.value())
            recurseMapReferences(componentId, subLayer);
    }
}

void Design::recurseUnmapReferences(const std::string &componentId, const octopus::Layer &layer) {
    if (layer.type == octopus::Layer::Type::COMPONENT_REFERENCE /* && layer.componentId.has_value() */) {
        // TODO referenceMap[layer.componentId.value()].erase(ComponentLayerId(componentId, layer.id));
    }
    if (layer.layers.has_value()) {
        for (const octopus::Layer &subLayer : layer.layers.value())
            recurseUnmapReferences(componentId, subLayer);
    }
}

}
