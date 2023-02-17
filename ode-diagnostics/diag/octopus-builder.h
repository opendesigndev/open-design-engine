
#pragma once

#include <octopus/octopus.h>
#include <ode-logic.h>

namespace ode {

namespace octopus_builder {

enum StrokePosition {
    OUTER,
    CENTER,
    INNER
};

class ShapeLayer : public octopus::Layer {

public:
    ShapeLayer();
    ShapeLayer(double x, double y);
    ShapeLayer(double x, double y, double w, double h);
    ShapeLayer &setOpacity(double opacity);
    ShapeLayer &setBlendMode(octopus::BlendMode blendMode);
    ShapeLayer &setPath(const std::string &path);
    ShapeLayer &setColor(const Color &color);
    ShapeLayer &setImage(const std::string &filename);
    ShapeLayer &addStroke(StrokePosition pos, const Color &color);
    ShapeLayer &addStroke(StrokePosition pos, double size, const Color &color);
    ShapeLayer &addEffect(const octopus::Effect &effect);

};

class TextLayer : public octopus::Layer {

public:
    explicit TextLayer(const std::string &text);
    TextLayer &setOpacity(double opacity);
    TextLayer &setBlendMode(octopus::BlendMode blendMode);
    TextLayer &setColor(const Color &color);
    TextLayer &addEffect(const octopus::Effect &effect);

};

class GroupLayer : public octopus::Layer {

public:
    GroupLayer();
    GroupLayer &setOpacity(double opacity);
    GroupLayer &setBlendMode(octopus::BlendMode blendMode);
    GroupLayer &add(const octopus::Layer &child);
    GroupLayer &addEffect(const octopus::Effect &effect);

};

class MaskGroupLayer : public GroupLayer {

public:
    MaskGroupLayer(octopus::MaskBasis basis, const octopus::Layer &mask);
    MaskGroupLayer &setMaskChannels(const double channels[5]);
    MaskGroupLayer &setMaskChannels(double a, double b, double c, double d, double e);

};

octopus::Octopus buildOctopus(const std::string &id, const octopus::Layer &root, double width, double height);
octopus::Octopus buildOctopus(const std::string &id, const octopus::Layer &root);

}

}
