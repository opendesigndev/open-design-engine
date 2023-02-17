
#pragma once

#include <optional>

#include <ode-essentials.h>
#include <ode-logic.h>

#include "RGIRenderedNode.h"
#include "RGIRenderGraph.h"

using namespace ode;

struct RGILoadedOctopus {
    /// The file path of the Octopus file loaded.
    FilePath filePath;
    /// The artboard, root layer of the design.
    ComponentPtr artboard;

    /// The constructed render graph for the loaded octopus image.
    RGIRenderGraph renderGraph;
    /// The rendered nodes of the graph, incl. their output bitmaps.
    RGIRenderedNodes renderedNodes;

    /// The constructed render graph for the loaded octopus image - alternative version.
    RGIRenderGraph renderGraphAltVersion;
    /// The rendered nodes of the graph, incl. their output bitmaps - alternative version.
    RGIRenderedNodes renderedNodesAltVersion;

    /// String representation of the graphviz render graph.
    std::string graphVizStr;
    //! Pointers to all unique layers.
    std::vector<const octopus::Layer *> layers;

    void clear();
    bool isLoaded() const;
    /// Get  result node.
    const RGIRenderedNode *resultNode() const;
    /// Get  result node bitmap.
    BitmapPtr resultBitmap() const;
    /// Get  result output image placement.
    std::optional<ScaledBounds> resultPlacement() const;
    /// Reset the vector of layers for this Octopus file.
    void resetLayers();
};
