
#pragma once

#include <ode-logic.h>
#include <ode-graphics.h>

// Forward declaration
struct DesignEditorLoadedOctopus;

using namespace ode;

class DesignEditorOctopusLoader {
public:
    struct Input {
        const FilePath octopusPath;
        const FilePath imageDirectory;
        const FilePath fontDirectory;
        bool ignoreValidation = false;

        inline bool isValid() const;
    };

    static bool readOctopusFile(const Input &input, GraphicsContext &gc, DesignEditorLoadedOctopus &oOctopus);
};
