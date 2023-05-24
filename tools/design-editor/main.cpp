
#include <optional>

#include <ode-logic.h>

#include "ode/DesignEditorWindow.h"

namespace {

#ifdef ODE_DEBUG
    #define BUILD_TYPE "Debug"
#endif
#ifdef ODE_RELEASE
    #define BUILD_TYPE "Release"
#endif
#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
    #define CONFIG_SKIA_GPU "enabled"
#else
    #define CONFIG_SKIA_GPU "disabled"
#endif

// TODO: Move this command line functionality and unify with other tools that have similar input?
/// Parsed command line input.
struct CommandLineInput {
    ode::FilePath octopusPath;
    ode::FilePath imageDirectory;
    ode::FilePath fontDirectory;
};

/// Parse command line input and return it. Or print version info (if specified) and return nullopt.
std::optional<CommandLineInput> parseCommandLineArguments(int argc, const char *const *argv) {
    CommandLineInput input;

    bool argEnd = false;

    for (int i = 1; i < argc; ++i) {
        std::string curArg(argv[i]);
        if (!argEnd) {
            bool done = true;
            if (curArg == "-i" || curArg == "--input" || curArg == "--octopus") {
                if (i+1 < argc) {
                    input.octopusPath = argv[++i];
                }
            } else if (curArg == "--image-assets" || curArg == "--assetpath" || curArg == "--bitmaps") {
                if (i+1 < argc)
                    input.imageDirectory = argv[++i];
            } else if (curArg == "--font-assets" || curArg == "--fontpath" || curArg == "--fonts") {
                if (i+1 < argc)
                    input.fontDirectory = argv[++i];
            } else if (curArg == "--") {
                argEnd = true;
            } else if (*argv[i] == '-') {
                fprintf(stderr, "Unknown setting: %s\n", argv[i]);
            } else {
                done = false;
            }
            if (done) {
                continue;
            }
        }
        if (input.octopusPath.empty()) {
            input.octopusPath = std::move(curArg);
        }
    }

    return input;
}

} // namespace

/// main - design-editor entry point
int main(int argc, const char *const *argv) {
    const std::optional<CommandLineInput> commandLineInputOpt = parseCommandLineArguments(argc, argv);

    DesignEditorWindow &window = DesignEditorWindow::getInstance();
    if (commandLineInputOpt.has_value()) {
        window.setImageDirectory(commandLineInputOpt->imageDirectory);
        window.setFontDirectory(commandLineInputOpt->fontDirectory);

        if (!commandLineInputOpt->octopusPath.empty()) {
            window.readOctopusFile(commandLineInputOpt->octopusPath);
        } else {
            window.createEmptyDesign();
        }
    }

    return window.display();
}
