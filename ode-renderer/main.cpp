
#include <cstdio>
#include <string>
#include <octopus/octopus.h>
#include <octopus/parser.h>
#include <octopus/validator.h>
#include <ode-essentials.h>
#include <ode-logic.h>
#include <ode-graphics.h>
#include <ode-media.h>
#include "ode/optimized-renderer/render.h"

#ifdef ODE_DEBUG
    #define BUILD_TYPE "Debug"
    #error Debug build
#endif
#ifdef ODE_RELEASE
    #define BUILD_TYPE "Release"
#else
    #error Debug build
#endif
#ifdef ODE_RASTERIZER_TEXTURE_SUPPORT
    #define CONFIG_SKIA_GPU "enabled"
#else
    #define CONFIG_SKIA_GPU "disabled"
#endif

using namespace ode;

// TODO MOVE
static std::string visualizeErrorPosition(const std::string &str, int position, const std::string &indent) {
    int strLen = int(str.size());
    if (position >= strLen)
        return std::string();
    std::string output;
    int caretPos = 0;
    if (position <= 32) {
        if (strLen > 64)
            output = indent+str.substr(0, 64)+"...";
        else
            output = indent+str.substr(0, strLen);
        caretPos = position;
    } else {
        if (strLen-(position-32) > 64)
            output = indent+"..."+str.substr(position-32, 64)+"...";
        else
            output = indent+"..."+str.substr(position-32, strLen-(position-32));
        caretPos = 35;
    }
    for (char &c : output) {
        if (c == '\r' || c == '\n')
            c = ' ';
    }
    output.push_back('\n');
    output += indent;
    for (int i = 0; i < caretPos; ++i)
        output.push_back(' ');
    output.push_back('^');
    output.push_back('\n');
    return output;
}

int main(int argc, const char *const *argv) {

    FilePath octopusPath;
    FilePath imageDirectory;
    FilePath fontDirectory;
    FilePath outputImagePath;
    bool ignoreValidation = false;

    bool octopusPathSet = false;
    bool argEnd = false;
    for (int i = 1; i < argc; ++i) {
        std::string curArg(argv[i]);
        if (!argEnd) {
            bool done = true;
            if (curArg == "-i" || curArg == "--input" || curArg == "--octopus") {
                if (i+1 < argc) {
                    octopusPath = argv[++i];
                    octopusPathSet = true;
                }
            } else if (curArg == "-o" || curArg == "--output") {
                if (i+1 < argc)
                    outputImagePath = argv[++i];
            } else if (curArg == "--image-assets" || curArg == "--assetpath" || curArg == "--bitmaps") {
                if (i+1 < argc)
                    imageDirectory = argv[++i];
            } else if (curArg == "--font-assets" || curArg == "--fontpath" || curArg == "--fonts") {
                if (i+1 < argc)
                    fontDirectory = argv[++i];
            } else if (curArg == "--ignore-validation") {
                ignoreValidation = true;
            } else if (curArg == "--version") {
                puts(
                    "\nOPEN DESIGN ENGINE v" ODE_STRINGIZE(ODE_VERSION) " by Ceros\n\n"
                    "      Build version: " ODE_STRINGIZE(ODE_BUILD_COMMIT_TAG) "\n"
                    "         Build date: " ODE_STRINGIZE(ODE_BUILD_DATE) "\n"
                    "         Build type: " BUILD_TYPE "\n"
                    "    Octopus version: " OCTOPUS_VERSION ", manifest " OCTOPUS_MANIFEST_VERSION "\n"
                    "           Skia GPU: " CONFIG_SKIA_GPU "\n"
                );
                return 0;
            } else if (curArg == "--") {
                argEnd = true;
            } else if (*argv[i] == '-') {
                fprintf(stderr, "Unknown setting: %s\n", argv[i]);
            } else
                done = false;
            if (done)
                continue;
        }
        if (!octopusPathSet) {
            octopusPath = (std::string &&) curArg;
            octopusPathSet = true;
        } else
            outputImagePath = (std::string &&) curArg;
    }

    if (!octopusPathSet) {
        puts("Usage: ode-renderer octopus.json output.png");
        return 0;
    }

    std::string octopusJson;
    if (!readFile(octopusPath, octopusJson)) {
        fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) octopusPath).c_str());
        return 1;
    }

    octopus::Octopus octopusData;
    if (octopus::Parser::Error parseError = octopus::Parser::parse(octopusData, octopusJson.c_str())) {
        fprintf(stderr, "Failed to parse Octopus file \"%s\"\n", ((const std::string &) octopusPath).c_str());
        fprintf(stderr, "    %s at position %d\n%s", parseError.typeString(), parseError.position, visualizeErrorPosition(octopusJson, parseError.position, "    ").c_str());
        fprintf(stderr, "Required Octopus spec is v%s\n", OCTOPUS_VERSION);
        return 1;
    }

    std::set<std::string> layerIds;
    std::string validationError;
    if (!octopus::validate(octopusData, layerIds, &validationError)) {
        fprintf(stderr, "Invalid Octopus: %s\n", validationError.c_str());
        if (!ignoreValidation)
            return 1;
    }

    if (imageDirectory.empty())
        imageDirectory = octopusPath.parent();
    if (fontDirectory.empty())
        fontDirectory = octopusPath.parent();
    if (outputImagePath.empty())
        outputImagePath = (const std::string &) octopusPath+".png";

    FontBasePtr fontBase(new FontBase);
    fontBase->setFontDirectory(fontDirectory);

    Component artboard;
    artboard.setFontBase(fontBase);
    if (DesignError error = artboard.initialize((octopus::Octopus &&) octopusData)) {
        fprintf(stderr, "Failed to initialize component\n");
        return 1;
    }

    Result<Rendexptr, DesignError> renderExpression = artboard.assemble();
    if (renderExpression.failure()) {
        fprintf(stderr, "Failed to assemble render expression\n");
        return 1;
    }

    GraphicsContext gc(GraphicsContext::OFFSCREEN);
    if (!gc) {
        fprintf(stderr, "Failed to establish OpenGL context\n");
        return 1;
    }

    Renderer renderer(gc);
    ImageBase imageBase(gc);
    imageBase.setImageDirectory(imageDirectory);
    UnscaledBounds componentBounds;
    if (artboard.getOctopus().dimensions.has_value()) {
        componentBounds.b.x = artboard.getOctopus().dimensions.value().width;
        componentBounds.b.y = artboard.getOctopus().dimensions.value().height;
    } else if (artboard.getOctopus().content.has_value()) {
        if (Result<LayerBounds, DesignError> contentBounds = artboard.getLayerBounds(artboard.getOctopus().content->id))
            componentBounds = contentBounds.value().bounds;
    } else {
        fprintf(stderr, "Nothing to render?\n");
        return 1;
    }
    const double scale = 1;
    PlacedImagePtr image = render(renderer, imageBase, artboard, renderExpression.value(), scale, outerPixelBounds(scaleBounds(componentBounds, scale)), 0);
    if (!image) {
        fprintf(stderr, "Failed to render\n");
        return 1;
    }

    if (BitmapPtr bitmap = image->asBitmap()) {
        if (isPixelPremultiplied(bitmap->format()))
            bitmapUnpremultiply(*bitmap);
        savePng(outputImagePath, *bitmap);
    } else {
        fprintf(stderr, "Internal error 15\n");
        return 1;
    }

    return 0;

}
