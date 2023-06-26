
#pragma once

#include <ode/api-base.h>
#include <ode-essentials.h>
#include "../logic-api.h"

namespace ode {

/// A function type for operating over images, specified by their path and byte buffer.
using ImageFunction = std::function<ODE_Result(ODE_StringRef, ODE_MemoryBuffer &)>;

/// Load a design from the specified octopus file.
ODE_Result loadDesignFromOctopusFile(ODE_DesignHandle *design, const FilePath &path, const ImageFunction &imageLoader, ODE_ParseError *parseError);
/// Save the specified design to an octopus file.
ODE_Result saveDesignToOctopusFile(ODE_DesignHandle design, const FilePath &path, const ImageFunction &imageExporter);

}
