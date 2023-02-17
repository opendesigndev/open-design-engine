
#include "graphviz.h"

#include <cstdio>
#include <queue>
#include <set>
#include <ode/render-expressions/render-expressions.h>

namespace ode {

namespace debug {

static std::string gvNodeName(const Rendexpr *ptr) {
    char buffer[64];
    sprintf(buffer, "E%p", ptr);
    return buffer;
}

static std::string gvEdge(const Rendexpr *a, const Rendexpr *b, const char *label = nullptr) {
    if (!(a && b))
        return std::string();
    return "    "+gvNodeName(a)+" -> "+gvNodeName(b)+(label ? " [label=\""+std::string(label)+"\"];\n" : ";\n");
}

static std::string gvLayerSuffix(const octopus::Layer *layer) {
    if (layer) {
        std::string layerId = layer->id;
        if (layerId.length() > 12)
            layerId = layerId.substr(0, 8);
        return " "+layerId;
    }
    return std::string();
}

std::string generateGraphviz(const Rendexptr &root) {
    std::string dot = "digraph rendexpr {\n";
    std::set<const Rendexpr *> visited;
    std::queue<const Rendexpr *> queue;
    queue.push(root.get());
    int i = 0;

    while (!queue.empty()) {
        const Rendexpr *node = queue.front();
        queue.pop();
        if (!(node && visited.insert(node).second))
            continue;

        dot += "    "+gvNodeName(node)+" [label=\"";

        switch (node->type) {
            #define VISIT_NODE(T) \
                dot += renderExpressionTypeShortName(node->type)+gvLayerSuffix(node->getLayer())+" ["+std::to_string(i)+"]\"];\n"
            #define VISIT_CHILD(T, m) \
                dot += gvEdge(node, (queue.push(static_cast<const T *>(node)->m.get()), static_cast<const T *>(node)->m.get()), #m)
            RENDER_EXPRESSION_CASES(VISIT_NODE, VISIT_CHILD)
            default:
                ODE_ASSERT(!"Unrecognized expression type");
                dot += "??? ["+std::to_string(i)+"]\"];\n";
        }

        ++i;
    }
    dot += "}\n";
    return dot;
}

}

}
