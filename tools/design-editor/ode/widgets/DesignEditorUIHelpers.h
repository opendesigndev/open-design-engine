
#pragma once

#include <string>

#include <imgui.h>

#include <octopus/octopus.h>
#include <ode/logic-api.h>

std::string layerTypeToShortString(ODE_LayerType layerType);
std::string layerTypeToString(ODE_LayerType layerType);

const octopus::Layer *findLayer(const octopus::Layer &layer, const std::string &layerId);
std::optional<std::string> findParentLayerId(const ODE_LayerList &layerList, const ODE_StringRef &layerId);

std::optional<std::string> findAvailableLayerId(const std::string &prefix, const ODE_LayerList &layerList);

ImVec4 toImColor(const octopus::Color &color);
octopus::Color toOctopusColor(const ImVec4 &color);

bool isImGuiMultiselectKeyModifierPressed();
