
#pragma once

#include <string>

void AddNodeTitleBar(const std::string &title);

void AddInputAttribute(int i, const char *attributeName);

void AddOutputAttribute(int i, const char *attributeName);

std::string NodeEnd(size_t nodeIndex);
