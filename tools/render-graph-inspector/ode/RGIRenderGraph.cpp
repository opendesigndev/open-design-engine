
#include "RGIRenderGraph.h"

#include <queue>
#include <set>

RGIRenderGraph::RGIRenderGraph() :
    RGIRenderGraph(nullptr) {
}

RGIRenderGraph::RGIRenderGraph(Rendexptr root_) :
    root(root_) {
    reinitialize(root);
}

RGIRenderGraph::~RGIRenderGraph() {
}

void RGIRenderGraph::reinitialize(Rendexptr newRoot) {
    root = newRoot;

    recalculateNodeParameters();
}

void RGIRenderGraph::clear() {
    root = nullptr;
    nodeIndices.clear();
    nodeDepths.clear();
    nodeChildCount.clear();
}

bool RGIRenderGraph::empty() const {
    return root == nullptr;
}

const Rendexptr RGIRenderGraph::getRoot() const {
    return root;
}

size_t RGIRenderGraph::getNodesCount() const {
    return nodeDepths.size();
}

int RGIRenderGraph::getNodeIndex(const Rendexpr *node) const {
    return (nodeIndices.find(node) == nodeIndices.end())
        ? -1
        : static_cast<int>(nodeIndices.at(node));
}

int RGIRenderGraph::getNodeDepth(const Rendexpr *node) const {
    return (nodeDepths.find(node) == nodeDepths.end())
        ? -1
        : static_cast<int>(nodeDepths.at(node));
}

size_t RGIRenderGraph::getMaxNodeDepth() const {
    size_t maxNodeDepth = 0;
    for (const std::pair<const Rendexpr *, size_t> nodeDepthPair : nodeDepths) {
        if (maxNodeDepth < nodeDepthPair.second) {
            maxNodeDepth = nodeDepthPair.second;
        }
    }
    return maxNodeDepth;
}

size_t RGIRenderGraph::getNodesChildrenCount(const Rendexpr *node) const {
    return (nodeChildCount.find(node) == nodeChildCount.end())
        ? 0
        : nodeChildCount.at(node);
}

void RGIRenderGraph::recalculateNodeParameters() {
    nodeIndices.clear();
    nodeDepths.clear();
    nodeChildCount.clear();

    if (root == nullptr) {
        return;
    }

    int nodeIndex = 0;
    nodeDepths[root.get()] = 0;

    std::set<const Rendexpr *> visited;
    std::queue<const Rendexpr *> queue;
    queue.push(root.get());

    while (!queue.empty()) {
        const Rendexpr *node = queue.front();
        queue.pop();
        if (!(node && visited.insert(node).second)) {
            continue;
        }

        switch (node->type) {
            #define VISIT_NODE(T) \
                if (nodeChildCount.find(static_cast<const T *>(node)) == nodeChildCount.end()) { nodeChildCount[static_cast<const T *>(node)] = 0; } \
                nodeIndices[static_cast<const T *>(node)] = nodeIndex++;

            #define VISIT_CHILD(T, m) \
                queue.push(static_cast<const T *>(node)->m.get()); \
                if (node != nullptr && static_cast<const T *>(node)->m.get() != nullptr && nodeDepths.find(static_cast<const T *>(node)->m.get()) == nodeDepths.end()) { \
                    nodeDepths[static_cast<const T *>(node)->m.get()] = nodeDepths[node] + 1; \
                } \
                nodeChildCount[static_cast<const T *>(node)]++;

            RENDER_EXPRESSION_CASES(VISIT_NODE, VISIT_CHILD)
        }
    }
}
