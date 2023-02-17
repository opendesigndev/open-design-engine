
#pragma once

#include <octopus/layer.h>
#include "TransformationMatrix.h"

namespace ode {

struct LayerInstanceSpecifier {
    const octopus::Layer *layer = nullptr;
    TransformationMatrix parentTransform;
    double parentFeatureScale = 1;

    constexpr const octopus::Layer &operator*() const {
        return *layer;
    }
    constexpr const octopus::Layer *operator->() const {
        return layer;
    }
    constexpr explicit operator bool() const {
        return layer != nullptr;
    }
};

}
