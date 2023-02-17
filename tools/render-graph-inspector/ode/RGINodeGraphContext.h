
#pragma once

#include <vector>

struct RGINodeGraphContext {
    /// Initialized flag.
    bool intialized = false;
    /// All selected node IDs.
    std::vector<int> selectedNodes = {};
    /// Selection string for textfield selection (layers).
    char layerSelectionStr[40] = {};
    /// Selection string for textfield selection (nodes).
    char nodesSelectionStr[40] = {};
    /// The mode of nodes selection in the graph.
    enum class SelectionMode {
        INIT_ROOT_NODE, /// Initial selection of the root node
        MOUSE,
        SEARCH_LAYER,
        SEARCH_NODES,
    } selectionMode = SelectionMode::MOUSE;

    void clear();

    bool isInitialSelection() const;
    bool isMouseSelection() const;
    bool isKeyboardSelection() const;
    bool isLayerSearchSelection() const;
    bool isNodesSearchSelection() const;
};
inline bool operator==(const RGINodeGraphContext &a, const RGINodeGraphContext &b) {
    return a.intialized == b.intialized && a.selectedNodes == b.selectedNodes;
}
inline bool operator!=(const RGINodeGraphContext &a, const RGINodeGraphContext &b) {
    return !(a == b);
}
