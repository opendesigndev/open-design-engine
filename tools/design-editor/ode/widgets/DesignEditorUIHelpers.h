
#pragma once

#include <string>

#include <imgui.h>

#include <octopus/octopus.h>
#include <ode/logic-api.h>

std::string layerTypeToShortString(ODE_LayerType layerType);
std::string layerTypeToString(ODE_LayerType layerType);

/// Find a layer by ID in the subtree of the specified layer
const octopus::Layer *findLayer(const octopus::Layer &layer, const std::string &layerId);
/// Find parent layer and get its ID
std::optional<std::string> findParentLayerId(const ODE_LayerList &layerList, const ODE_StringRef &layerId);

/// Find an available layer ID in the form prefix_{i} for int i
std::optional<std::string> findAvailableLayerId(const std::string &prefix, const ODE_LayerList &layerList);

/// Color conversion from  Octopus to ImGui
ImVec4 toImColor(const octopus::Color &color);
/// Color conversion from  ImGUi to Octopus
octopus::Color toOctopusColor(const ImVec4 &color);

bool isImGuiMultiselectKeyModifierPressed();

/// Detect if there is an intersection between the two rectangles
bool isRectangleIntersection(const ODE_Rectangle &r1, const ODE_Rectangle &r2);

/// Get the bounding rectangle of the specified layer
ODE_Rectangle getBoundingRectangle(const ODE_LayerMetrics &layerMetrics);
