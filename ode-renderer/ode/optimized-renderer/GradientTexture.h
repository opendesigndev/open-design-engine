
#pragma once

#include <octopus/octopus.h>
#include <ode-graphics.h>

namespace ode {

class GradientTexture {

public:
    GradientTexture();
    bool initialize(const octopus::Gradient &gradient);
    bool initialize(const std::vector<octopus::Gradient::ColorStop> &colorStops);
    void bind(int unit) const;
    Vector2f transformation() const;

private:
    Texture2D texture;
    double remap[2];

};

}
