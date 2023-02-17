
#pragma once

#include <map>

#include <ode-logic.h>

using namespace ode;

/// A representation of a render graph of Render Expressions
class RGIRenderGraph {
public:
    RGIRenderGraph();
    explicit RGIRenderGraph(Rendexptr root_);
    ~RGIRenderGraph();

    /// Reinitialize and recalculate graph variables
    void reinitialize(Rendexptr newRoot);
    /// Clear the render graph - erase its content.
    void clear();

    /// Is empty
    bool empty() const;
    /// Get root node expresion
    const Rendexptr getRoot() const;
    /// Get count of all nodes in the graph
    size_t getNodesCount() const;
    /// Get index of the specified node in the graph
    int getNodeIndex(const Rendexpr *node) const;
    /// Get deph of the specified node in the graph
    int getNodeDepth(const Rendexpr *node) const;
    /// Get maximum node depth
    size_t getMaxNodeDepth() const;
    /// Return the count of child nodes for the spicified node.
    size_t getNodesChildrenCount(const Rendexpr *node) const;

private:
    Rendexptr root = nullptr;

    void recalculateNodeParameters();

    std::map<const Rendexpr *, int> nodeIndices;
    std::map<const Rendexpr *, size_t> nodeDepths;
    std::map<const Rendexpr *, size_t> nodeChildCount;
};
