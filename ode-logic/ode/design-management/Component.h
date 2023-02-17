
#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>
#include <nonstd/optional.hpp>
#include <octopus/octopus.h>
#include <octopus-manifest/component.h>
#include <ode-essentials.h>
#include <ode-rasterizer.h>
#include "../textify/textify.h"
#include "../core/bounds.h"
#include "../core/LayerBounds.h"
#include "../core/LayerMetrics.h"
#include "../core/LayerInstanceSpecifier.h"
#include "../render-expressions/RenderExpression.h"
#include "../render-assembly/assembly.h"
#include "../animation/DocumentAnimation.h"
#include "DesignError.h"
#include "LayerInstance.h"

namespace ode {

// TODO MOVE THESE:
class ResourceBase {
public:
    const std::string *getOctopus(const octopus::ResourceLocation);
};

// TODO MOVE THESE:
// Used to find "symbol" components by id
class ComponentRetriever {
public:
    virtual ~ComponentRetriever() = default;
    virtual const octopus::Octopus *getComponentOctopus(const std::string &id);
    virtual const octopus::Layer *getComponentLayerOctopus(const std::string &componentId, const std::string &layerId);
};

class Component;

typedef std::unique_ptr<Component> ComponentPtr;

/// Manages a design component
class Component {

public:
    static Result<ComponentPtr, DesignError> create(const octopus::Component &componentManifest, ResourceBase *resourceBase);

    Component();
    explicit Component(const std::string &id);
    Component(const Component &) = delete;
    Component(Component &&orig) = default;
    Component &operator=(const Component &) = delete;
    Component &operator=(Component &&orig) = default;

    inline const std::string &getId() const { return id; }
    /// Returns the revision number which increments after each modification
    inline int revision() const { return rev; }

    DesignError initialize(const octopus::Octopus &octopus);
    DesignError initialize(octopus::Octopus &&octopus);
    DesignError setFontBase(const FontBasePtr &fontBase);
    DesignError setAnimation(const DocumentAnimation &animation);
    DesignError setPosition(const Vector2d &position);
    DesignError setName(const std::string &name);

    /// Builds layer data immediately
    DesignError prebuild();

    /// Replaces the root layer
    DesignError setRootLayer(const octopus::Layer &layer);
    /// Adds a new layer as the last child of parent
    DesignError addLayer(const std::string &parent, const std::string &before, const octopus::Layer &layer);
    /// Removes a layer from the component
    DesignError removeLayer(const std::string &id);
    /// Permanently modifies a layer with the specified layerChange
    DesignError modifyLayer(const std::string &id, const octopus::LayerChange &layerChange);
    /// Informs component reference that the master component or one of its layers has changed
    DesignError notifyReference(const std::string &referencelayerId, const std::string &masterLayerId, const octopus::LayerChange *layerChange);

    /// Returns the current Octopus representation with unresolved references
    const octopus::Octopus &getOctopus() const;
    /// Compiles the complete Octopus representation with references replaced by their content
    Result<octopus::Octopus, DesignError> buildOctopus();
    /// Returns the pointer to the Octopus representation of a given layer
    Result<const octopus::Layer *, DesignError> getLayerOctopus(const std::string &id);
    /// Compiles the resolved Octopus representation of a given layer, may exclude its child layers
    Result<octopus::Octopus, DesignError> buildLayerOctopus(const std::string &id, bool recursive);
    /// Returns a layer's bounds data
    Result<LayerBounds, DesignError> getLayerBounds(const std::string &id);
    /// Returns the relevant metrics for a given layer
    Result<LayerMetrics, DesignError> getLayerMetrics(const std::string &id);
    /// Returns the pointer to Rasterizer Shape of a given SHAPE layer
    Result<Rasterizer::Shape *, DesignError> getLayerShape(const std::string &id);
    /// Returns the Textify TextShapeHandle of a given TEXT layer
    Result<textify::TextShapeHandle, DesignError> getLayerTextShape(const std::string &id);
    /// Returns animations of a given layer
    Result<const DocumentAnimation *, DesignError> getAnimation(const std::string &id);
    /// Returns value of a specific animation at a given time
    Result<LayerAnimation::Keyframe, DesignError> getAnimationValue(int index, double time) const;
    /// Attempts to find a layer by its position within the component - returns the tompost if multiple
    std::string identifyLayer(const Vector2d &position, double radius);

    /// Adds component's missing fonts to the set of names
    void listMissingFonts(std::set<std::string> &names) const;

    /// Assembles the render tree for the component
    Result<Rendexptr, DesignError> assemble();
    /// Assembles the render tree for a specific layer within the component
    Result<Rendexptr, DesignError> assembleLayer(const std::string &id);

private:

    std::string id;
    std::string name;
    Vector2d position;
    octopus::Octopus octopus;
    FontBasePtr fontBase;

    /// Component revision number - increments after every modification
    int rev = 0;
    bool buildComplete = false;
    std::map<std::string, LayerInstance> instances;
    std::set<std::string> subComponents; // direct, layer ID's

    // TODO remove when animations are indexed by id
    DocumentAnimation allAnimations;

    static int assemblyFlags(const LayerInstance &instance);

    LayerInstance *findInstance(const std::string &id);

    DesignError requireBuild();
    DesignError rebuild();
    Result<LayerInstance *, DesignError> rebuildSubtree(octopus::Layer *layer, const TransformationMatrix &parentTransform, double parentFeatureScale, const std::string &parentId);
    void addSubtreeAnimation(const LayerAnimation &animation, const std::list<octopus::Layer> &layers);
    RendexSubtree assembleLayer(const LayerInstance &instance, const nonstd::optional<octopus::MaskBasis> &maskBasis);

    bool identifyLayer(std::string &id, const LayerInstance &instance, const Vector2d &position, double radius);

};

}
