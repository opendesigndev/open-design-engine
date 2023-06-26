
#pragma once

#include <ode/api-base.h>
#include <ode-essentials.h>
#include "../logic-api.h"

namespace ode {

using ImageFunction = std::function<ODE_Result(ODE_StringRef, ODE_MemoryBuffer &)>;

ODE_Result loadOctopusDesignFromFile(ODE_DesignHandle *design, const FilePath &path, const ImageFunction &imageLoader, ODE_ParseError *parseError);

ODE_Result saveOctopusDesignToFile(ODE_DesignHandle design, ODE_StringRef path, const ImageFunction &imageExporter);

}
