
#include "RGINodeGraphContext.h"

#include <cstring>

void RGINodeGraphContext::clear() {
    intialized = false;
    selectedNodes.clear();
    memset(&(layerSelectionStr[0]), 0, sizeof(layerSelectionStr));
    memset(&(nodesSelectionStr[0]), 0, sizeof(nodesSelectionStr));
    selectionMode = SelectionMode::MOUSE;
}

bool RGINodeGraphContext::isInitialSelection() const {
    return selectionMode == SelectionMode::INIT_ROOT_NODE;
}

bool RGINodeGraphContext::isMouseSelection() const {
    return selectionMode == SelectionMode::MOUSE;
}

bool RGINodeGraphContext::isLayerSearchSelection() const {
    return selectionMode == SelectionMode::SEARCH_LAYER;
}

bool RGINodeGraphContext::isNodesSearchSelection() const {
    return selectionMode == SelectionMode::SEARCH_NODES;
}
