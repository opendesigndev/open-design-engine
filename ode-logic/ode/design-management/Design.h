
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <octopus/octopus.h>
#include <octopus-manifest/octopus-manifest.h>
#include <ode-essentials.h>
#include "DesignError.h"
#include "Component.h"

namespace ode {

/** A design is a collection of components organized into pages etc.
 *  The class takes care of references between components and on-demand loading.
 */
class Design {

public:
    /** All components are private and cannot be accessed directly because modifications
     *  may cause side effects in other parts of the design via component references.
     *  Therefore, all component modifications are relayed through ComponentAccessor,
     *  which also resolves changes to their references.
     */
    class ComponentAccessor {

    public:
        inline ComponentAccessor() : design(nullptr), component(nullptr) { }
        inline ComponentAccessor(Design *design, Component *component) : design(design), component(component) { }
        // 1. call function on Component
        // 2. for each reference in referenceMap, notify reference's parent component
        // 3. for each reference of reference's parent component, do the same recursively
        DesignError setRootLayer(const octopus::Layer &layer);
        DesignError addLayer(const std::string &parent, const std::string &before, const octopus::Layer &layer);
        DesignError removeLayer(const std::string &id);
        DesignError modifyLayer(const std::string &id, const octopus::LayerChange &layerChange);
        DesignError transformLayer(const std::string &id, octopus::Fill::Positioning::Origin basis, const TransformationMatrix &transformation);

        DesignError setAnimation(const DocumentAnimation &animation);

        const std::string &getId() const;
        int revision() const;
        const octopus::Octopus &getOctopus() const;
        Result<const octopus::Layer *, DesignError> getLayerOctopus(const std::string &id);
        Result<octopus::Octopus, DesignError> buildOctopus() const; // "expanded"
        Result<octopus::Octopus, DesignError> buildLayerOctopus(const std::string &id, bool recursive) const; // "expanded"
        Result<LayerBounds, DesignError> getLayerBounds(const std::string &id) const;
        Result<LayerMetrics, DesignError> getLayerMetrics(const std::string &id);
        Result<LayerAnimation::Keyframe, DesignError> getAnimationValue(int index, double time) const;
        std::string identifyLayer(const Vector2d &position, double radius);
        // Additional Layer stuff here
        void listMissingFonts(std::set<std::string> &names) const;

        Result<Rendexptr, DesignError> assemble();

        inline Component *TEMP_GET_COMPONENT_DELETE_ME_ASAP() { return component; }

        explicit operator bool() const;

    private:
        Design *design;
        Component *component;

        /** Called (recursively) after any of the former
         *  referencelayerId - ID of the COMPONENT_REFERENCE layer whose master has been modified
         *  masterLayerId - ID of the changed / added / removed in master component. If empty, assumes the whole component has changed (e.g. root layer change)
         *  layerChange - pointer to the parameter from modifyLayer if available, otherwise null
         */
        void notifyReferences(Component *component, const std::string &masterLayerId, const octopus::LayerChange *layerChange);

    };

    struct Page {
        std::string id;
        std::string name;
        std::set<std::string> components;
    };

    static Result<Design, DesignError> loadManifest(const octopus::OctopusManifest &manifest);
    DesignError addComponent(const octopus::Component &component);

    // TODO modify to defer initialization
    DesignError addComponent(const octopus::Component &component, octopus::Octopus &&octopus);

    DesignError removeComponent(const ComponentAccessor &component);
    DesignError removeComponent(const std::string &id);

    ComponentAccessor getComponent(const std::string &id);

    void listComponents(std::set<std::string> &ids) const;
    void listMissingFonts(std::set<std::string> &names) const;

private:
    struct ComponentLayerId {
        std::string component;
        std::string layer;
    };

    // Source of truth
    std::string name;
    std::map<std::string, ComponentPtr> components; // only loaded components
    std::map<std::string, octopus::Component> protocomponents;
    std::vector<std::string> artboards;
    std::map<std::string, Page> pages;

    ResourceBase *resourceBase; // TODO

    // Derived
    std::map<std::string, std::set<ComponentLayerId> > referenceMap;

    Component *requireComponent(const std::string &id);
    void onComponentLoaded(const Component *component); // update derived

    void recurseMapReferences(const std::string &componentId, const octopus::Layer &layer);
    void recurseUnmapReferences(const std::string &componentId, const octopus::Layer &layer);

};

}
