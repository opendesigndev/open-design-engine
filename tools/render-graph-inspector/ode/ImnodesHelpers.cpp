
#include "ImnodesHelpers.h"

#include <imnodes.h>

void AddNodeTitleBar(const std::string &title) {
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(title.c_str());
    ImNodes::EndNodeTitleBar();
}

void AddInputAttribute(int i, const char *attributeName) {
    ImNodes::BeginInputAttribute(i);
    ImGui::Text("%s", attributeName);
    ImNodes::EndInputAttribute();
}

void AddOutputAttribute(int i, const char *attributeName) {
    ImNodes::BeginOutputAttribute(i);
    ImGui::Indent(10);
    ImGui::Text("%s", attributeName);
    ImNodes::EndOutputAttribute();
}

std::string NodeEnd(size_t nodeIndex) {
    return std::string(" [")+std::to_string(nodeIndex)+std::string("]");
}
