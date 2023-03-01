
#include "DesignEditorLoadedOctopus.h"

void DesignEditorLoadedOctopus::clear() {
    filePath = "";
    artboard.reset();
    layers.clear();
}

bool DesignEditorLoadedOctopus::isLoaded() const {
    return !filePath.empty();
}

ode::BitmapPtr DesignEditorLoadedOctopus::resultBitmap() const {
    return bitmap;
}

std::optional<ode::ScaledBounds> DesignEditorLoadedOctopus::resultPlacement() const {
    return placement;
}

void DesignEditorLoadedOctopus::resetLayers() {
    layers.clear();

    // TODO: DE
//    for (const DesignEditorRenderedNode &node : renderedNodes) {
//        const octopus::Layer *layer = node.node->getLayer();
//        if (layer != nullptr && !layer->id.empty()) {
//            layers.emplace_back(layer);
//        }
//    }

    std::sort(layers.begin(), layers.end());
    std::vector<const octopus::Layer *>::iterator ip = std::unique(layers.begin(), layers.end());
    layers.resize(std::distance(layers.begin(), ip));
}
