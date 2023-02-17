
#include <optional>

#include "ode/RGIWindow.h"

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

// TODO: (Matus) Move this command line functionality and unify with other tools that have similar input?
/// Parsed command line input.
struct CommandLineInput {
    ode::FilePath octopusPath;
    ode::FilePath imageDirectory;
    ode::FilePath fontDirectory;
    bool ignoreValidation = false;
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
            } else if (curArg == "--ignore-validation") {
                input.ignoreValidation = true;
            } else if (curArg == "--version") {
                puts(
                    "\nOPEN DESIGN ENGINE" /* " v" ODE_STRINGIZE(ODE_VERSION) */ " by Ceros\n\n" // put in when version is actually used
                    "      Build version: " ODE_STRINGIZE(ODE_BUILD_COMMIT) "\n"
                    "         Build date: " ODE_STRINGIZE(ODE_BUILD_DATE) "\n"
                    "         Build type: " BUILD_TYPE "\n"
                    "    Octopus version: " OCTOPUS_VERSION ", manifest " OCTOPUS_MANIFEST_VERSION "\n"
                    "           Skia GPU: " CONFIG_SKIA_GPU "\n"
                );
                return std::nullopt;
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

/// main - render-graph-inspector entry point
int main(int argc, const char *const *argv) {
    const std::optional<CommandLineInput> commandLineInputOpt = parseCommandLineArguments(argc, argv);

    RGIWindow &window = RGIWindow::getInstance();
    if (commandLineInputOpt.has_value()) {
        window.setImageDirectory(commandLineInputOpt->imageDirectory);
        window.setFontDirectory(commandLineInputOpt->fontDirectory);
        window.setIgnoreValidation(commandLineInputOpt->ignoreValidation);

        if (!commandLineInputOpt->octopusPath.empty()) {
            window.readOctopusFile(commandLineInputOpt->octopusPath);
        }
    }

    return window.display();
}
