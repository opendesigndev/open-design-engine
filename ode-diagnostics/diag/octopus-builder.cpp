
#include "octopus-builder.h"

namespace ode {

namespace octopus_builder {

ShapeLayer::ShapeLayer() : ShapeLayer(20, 20) { }

ShapeLayer::ShapeLayer(double x, double y) : ShapeLayer(x, y, 240, 160) { }

ShapeLayer::ShapeLayer(double x, double y, double w, double h) {
    static int i = 0;
    id = "SHAPE"+std::to_string(i++);
    type = octopus::Layer::Type::SHAPE;
    name = id;
    shape = octopus::Shape();
    shape->path = octopus::Path();
    shape->path->type = octopus::Path::Type::RECTANGLE;
    shape->path->rectangle = octopus::Rectangle { x, y, x+w, y+h };
    shape->fills.emplace_back();
    shape->fills.back().type = octopus::Fill::Type::COLOR;
    shape->fills.back().color = octopus::Color { 0, .5, 1, 1 };
}

ShapeLayer &ShapeLayer::setOpacity(double opacity) {
    this->opacity = opacity;
    return *this;
}

ShapeLayer &ShapeLayer::setBlendMode(octopus::BlendMode blendMode) {
    this->blendMode = blendMode;
    return *this;
}

ShapeLayer &ShapeLayer::setPath(const std::string &path) {
    shape->path->type = octopus::Path::Type::PATH;
    shape->path->geometry = path;
    shape->path->paths = std::nullopt;
    shape->path->rectangle = std::nullopt;
    return *this;
}

ShapeLayer &ShapeLayer::setColor(const Color &color) {
    octopus::Color octocolor;
    octocolor.r = color.r;
    octocolor.g = color.g;
    octocolor.b = color.b;
    octocolor.a = color.a;
    shape->fills.back().type = octopus::Fill::Type::COLOR;
    shape->fills.back().color = octocolor;
    return *this;
}

ShapeLayer &ShapeLayer::setImage(const std::string &filename) {
    shape->fills.back().type = octopus::Fill::Type::IMAGE;
    shape->fills.back().color = std::nullopt;
    shape->fills.back().gradient = std::nullopt;
    shape->fills.back().image = octopus::Image();
    shape->fills.back().image->ref.type = octopus::ImageRef::Type::PATH;
    shape->fills.back().image->ref.value = filename;
    return *this;
}

ShapeLayer &ShapeLayer::addStroke(StrokePosition pos, const Color &color) {
    return addStroke(pos, 8, color);
}

ShapeLayer &ShapeLayer::addStroke(StrokePosition pos, double size, const Color &color) {
    octopus::Color octocolor;
    octocolor.r = color.r;
    octocolor.g = color.g;
    octocolor.b = color.b;
    octocolor.a = color.a;
    octopus::Shape::Stroke stroke;
    stroke.fill.type = octopus::Fill::Type::COLOR;
    stroke.fill.color = octocolor;
    stroke.thickness = size;
    switch (pos) {
        case OUTER:
            stroke.position = octopus::Stroke::Position::OUTSIDE;
            break;
        case CENTER:
            stroke.position = octopus::Stroke::Position::CENTER;
            break;
        case INNER:
            stroke.position = octopus::Stroke::Position::INSIDE;
            break;
    }
    shape->strokes.push_back(stroke);
    return *this;
}

ShapeLayer &ShapeLayer::addEffect(const octopus::Effect &effect) {
    effects.push_back(effect);
    return *this;
}

TextLayer::TextLayer(const std::string &text) {
    id = text;
    type = octopus::Layer::Type::TEXT;
    name = id;
    this->text = octopus::Text();
    this->text->value = text;
    this->text->defaultStyle.font = octopus::Font();
    this->text->defaultStyle.font->postScriptName = "arial";
    this->text->defaultStyle.fontSize = 20;
    this->text->defaultStyle.fills = std::vector<octopus::Fill>();
    this->text->defaultStyle.fills->emplace_back();
    this->text->defaultStyle.fills->back().type = octopus::Fill::Type::COLOR;
    this->text->defaultStyle.fills->back().color = octopus::Color { .25, .25, .25, 1 };
}

TextLayer &TextLayer::setOpacity(double opacity) {
    this->opacity = opacity;
    return *this;
}

TextLayer &TextLayer::setBlendMode(octopus::BlendMode blendMode) {
    this->blendMode = blendMode;
    return *this;
}

TextLayer &TextLayer::setColor(const Color &color) {
    octopus::Color octocolor;
    octocolor.r = color.r;
    octocolor.g = color.g;
    octocolor.b = color.b;
    octocolor.a = color.a;
    this->text->defaultStyle.fills->back().type = octopus::Fill::Type::COLOR;
    this->text->defaultStyle.fills->back().color = octocolor;
    return *this;
}

TextLayer &TextLayer::addEffect(const octopus::Effect &effect) {
    effects.push_back(effect);
    return *this;
}

GroupLayer::GroupLayer() {
    static int i = 0;
    id = "GROUP"+std::to_string(i++);
    type = octopus::Layer::Type::GROUP;
    name = id;
    layers = std::list<octopus::Layer>();
}

GroupLayer &GroupLayer::setOpacity(double opacity) {
    this->opacity = opacity;
    return *this;
}

GroupLayer &GroupLayer::setBlendMode(octopus::BlendMode blendMode) {
    this->blendMode = blendMode;
    return *this;
}

GroupLayer &GroupLayer::add(const octopus::Layer &child) {
    layers->push_back(child);
    return *this;
}

GroupLayer &GroupLayer::addEffect(const octopus::Effect &effect) {
    effects.push_back(effect);
    return *this;
}

MaskGroupLayer::MaskGroupLayer(octopus::MaskBasis basis, const octopus::Layer &mask) {
    id = "MASK_"+id;
    type = octopus::Layer::Type::MASK_GROUP;
    name = id;
    maskBasis = basis;
    this->mask = new octopus::Layer(mask);
}

MaskGroupLayer &MaskGroupLayer::setMaskChannels(const double channels[5]) {
    maskChannels = std::array<double, 5> { channels[0], channels[1], channels[2], channels[3], channels[4] };
    return *this;
}

MaskGroupLayer &MaskGroupLayer::setMaskChannels(double a, double b, double c, double d, double e) {
    maskChannels = std::array<double, 5> { a, b, c, d, e };
    return *this;
}

octopus::Octopus buildOctopus(const std::string &id, const octopus::Layer &root, double width, double height) {
    octopus::Octopus document;
    document.version = OCTOPUS_VERSION;
    document.id = id;
    if (width > 0 && height > 0)
        document.dimensions = octopus::Dimensions { width, height };
    document.content = new octopus::Layer(root);
    return document;
}

octopus::Octopus buildOctopus(const std::string &id, const octopus::Layer &root) {
    return buildOctopus(id, root, 640, 480);
}

}

}
