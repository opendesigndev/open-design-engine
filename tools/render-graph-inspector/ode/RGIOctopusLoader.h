
#pragma once

#include <ode-graphics.h>

#include "RGIRenderedNode.h"

// Forward declaration
struct RGILoadedOctopus;

using namespace ode;

class RGIOctopusLoader {
public:
    struct Input {
        const FilePath octopusPath;
        const FilePath imageDirectory;
        const FilePath fontDirectory;
        bool ignoreValidation = false;

        inline bool isValid() const;
    };

    static bool readOctopusFile(const Input &input, GraphicsContext &gc, RGILoadedOctopus &oOctopus);
};
