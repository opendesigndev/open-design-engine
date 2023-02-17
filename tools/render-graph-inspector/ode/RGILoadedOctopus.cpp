
#include "RGILoadedOctopus.h"

void RGILoadedOctopus::clear() {
    filePath = "";
    artboard.reset();
    renderGraph.clear();
    renderedNodes.clear();
    renderGraphAltVersion.clear();
    renderedNodesAltVersion.clear();
    graphVizStr.clear();
    layers.clear();
}

bool RGILoadedOctopus::isLoaded() const {
    return !filePath.empty() && !renderGraph.empty() && !renderedNodes.empty();
}

const RGIRenderedNode *RGILoadedOctopus::resultNode() const {
    return renderedNodes.empty() ? nullptr : &renderedNodes.back();
}

ode::BitmapPtr RGILoadedOctopus::resultBitmap() const {
    const RGIRenderedNode *rn = resultNode();
    return rn == nullptr ? nullptr : rn->bitmap;
}

std::optional<ode::ScaledBounds> RGILoadedOctopus::resultPlacement() const {
    const RGIRenderedNode *rn = resultNode();
    return rn == nullptr ? std::nullopt : std::make_optional(rn->placement);
}

void RGILoadedOctopus::resetLayers() {
    layers.clear();

    for (const RGIRenderedNode &node : renderedNodes) {
        const octopus::Layer *layer = node.node->getLayer();
        if (layer != nullptr && !layer->id.empty()) {
            layers.emplace_back(layer);
        }
    }

    std::sort(layers.begin(), layers.end());
    std::vector<const octopus::Layer *>::iterator ip = std::unique(layers.begin(), layers.end());
    layers.resize(std::distance(layers.begin(), ip));
}
