
#pragma once

#include <string>
#include <octopus/octopus.h>

namespace ode {

// Used to find "symbol" components by id
class ComponentRetriever {
public:
    virtual ~ComponentRetriever() = default;
    virtual const octopus::Octopus *getComponentOctopus(const std::string &id);
    virtual const octopus::Layer *getComponentLayerOctopus(const std::string &componentId, const std::string &layerId);
};

}
