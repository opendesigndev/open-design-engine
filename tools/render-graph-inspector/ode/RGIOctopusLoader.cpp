
#include "RGIOctopusLoader.h"

// OD includes
#include <octopus/octopus.h>
#include <octopus/parser.h>
#include <octopus/validator.h>
#include <ode-renderer.h>
#include <ode-diagnostics.h>
#include <ode/optimized-renderer/render.h>
#include "ImnodesHelpers.h"
#include "RGILoadedOctopus.h"

namespace {

// TODO: Matus: MOVE and unify with ode-renderer-cli / main.cpp
static std::string visualizeErrorPosition(const std::string &str, int position, const std::string &indent) {
    const int strLen = int(str.size());
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

}


bool RGIOctopusLoader::Input::isValid() const {
    return !octopusPath.empty();
}


/*static*/ bool RGIOctopusLoader::readOctopusFile(const Input &input, GraphicsContext &gc, RGILoadedOctopus &oOctopus) {
    oOctopus.clear();

    if (!input.isValid()) {
        return false;
    }

    std::string octopusJson;
    if (!readFile(input.octopusPath, octopusJson)) {
        fprintf(stderr, "Failed to read file \"%s\"\n", ((const std::string &) input.octopusPath).c_str());
        return false;
    }

    octopus::Octopus octopusData;
    if (octopus::Parser::Error parseError = octopus::Parser::parse(octopusData, octopusJson.c_str())) {
        fprintf(stderr, "Failed to parse Octopus file \"%s\"\n", ((const std::string &) input.octopusPath).c_str());
        fprintf(stderr, "    %s at position %d\n%s", parseError.typeString(), parseError.position, visualizeErrorPosition(octopusJson, parseError.position, "    ").c_str());
        fprintf(stderr, "Required Octopus spec is v%s\n", OCTOPUS_VERSION);
        return false;
    }

    std::set<std::string> layerIds;
    std::string validationError;
    if (!octopus::validate(octopusData, layerIds, &validationError)) {
        fprintf(stderr, "Invalid Octopus: %s\n", validationError.c_str());
        if (!input.ignoreValidation) {
            return false;
        }
    }

    FontBasePtr fontBase(new FontBase);
    fontBase->setFontDirectory(input.fontDirectory.empty() ? input.octopusPath.parent() : input.fontDirectory);

    oOctopus.artboard = std::make_unique<Component>();
    oOctopus.artboard->setFontBase(fontBase);
    if (oOctopus.artboard->initialize(std::move(octopusData))) {
        fprintf(stderr, "Failed to initialize component\n");
        return false;
    }

    Result<Rendexptr, DesignError> renderExpression = oOctopus.artboard->assemble();
    if (renderExpression.failure()) {
        fprintf(stderr, "Failed to assemble render expression\n");
        return false;
    }

    oOctopus.filePath = input.octopusPath;
    oOctopus.renderGraph.reinitialize(renderExpression.value());
    oOctopus.renderGraphAltVersion.reinitialize(renderExpression.value());
    oOctopus.graphVizStr = debug::generateGraphviz(oOctopus.renderGraph.getRoot());

    UnscaledBounds componentBounds;
    if (oOctopus.artboard->getOctopus().dimensions.has_value()) {
        componentBounds.b.x = oOctopus.artboard->getOctopus().dimensions.value().width;
        componentBounds.b.y = oOctopus.artboard->getOctopus().dimensions.value().height;
    } else if (oOctopus.artboard->getOctopus().content.has_value()) {
        if (Result<LayerBounds, DesignError> contentBounds = oOctopus.artboard->getLayerBounds(oOctopus.artboard->getOctopus().content->id))
            componentBounds = contentBounds.value().bounds;
    } else {
        fprintf(stderr, "Nothing to render\n");
        return false;
    }
    PixelBounds pixelBounds = outerPixelBounds(scaleBounds(componentBounds, 1));

    Renderer defaultRenderer(gc);
//    Renderer newRenderer(gc);
//    newRenderer.setVersion("TBD");

    ImageBase imageBase(gc);
    imageBase.setImageDirectory(input.imageDirectory.empty() ? input.octopusPath.parent() : input.imageDirectory);

    const auto makeRenderHook = [](RGIRenderGraph &renderGraph, RGIRenderedNodes &renderedNodes) {
        return [&renderGraph, &renderedNodes](const Rendexpr *node, const PlacedImagePtr &image) {
            // Gather intermediate images
            if (image) {
                const BitmapPtr bitmap = image->asBitmap();
                if (bitmap) {
                    const int nodeIndex = renderGraph.getNodeIndex(node);
                    renderedNodes.emplace_back(RGIRenderedNode { nodeIndex+1, node, renderExpressionTypeShortName(node->type) + NodeEnd(nodeIndex+1), bitmap, image.bounds() });
                } else {
                    fprintf(stderr, "Internal error (image to bitmap conversion)\n");
                }
            }
        };
    };

    const PlacedImagePtr image = render(defaultRenderer, imageBase, *oOctopus.artboard, oOctopus.renderGraph.getRoot(), 1, pixelBounds, 0, makeRenderHook(oOctopus.renderGraph, oOctopus.renderedNodes));
    if (!image) {
        fprintf(stderr, "Failed to render\n");
        return false;
    }

//    const PlacedImagePtr imageNewRenderer = render(newRenderer, imageBase, *oOctopus.artboard, oOctopus.renderGraphAltVersion.getRoot(), 1, pixelBounds, 0, makeRenderHook(oOctopus.renderGraphAltVersion, oOctopus.renderedNodesAltVersion));
//    if (!imageNewRenderer) {
//        fprintf(stderr, "Failed to render\n");
//        return false;
//    }

    oOctopus.resetLayers();

    return true;
}
