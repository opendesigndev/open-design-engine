
#pragma once

#include <string>
#include <octopus-manifest/octopus-manifest.h>

namespace ode {

class ResourceBase {
public:
    inline const std::string *getOctopus(const octopus::ResourceLocation &) { return nullptr; }
};

}
