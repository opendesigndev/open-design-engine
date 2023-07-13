
// Generated by json-cpp-gen by Viktor Chlumsky
// https://github.com/Chlumsky/json-cpp-gen

#include "AnimationSerializer.h"

#ifndef JSON_CPP_SERIALIZE_DOUBLE
#include <cstdio>
#define JSON_CPP_SERIALIZE_DOUBLE(outBuffer, x) sprintf(outBuffer, "%.17g", x)
#endif

namespace ode {

AnimationSerializer::Error::operator AnimationSerializer::Error::Type() const {
    return type;
}

AnimationSerializer::Error::operator bool() const {
    return type != Error::OK;
}

const char *AnimationSerializer::Error::typeString() const {
    switch (type) {
        case Error::OK:
            return "OK";
        case Error::UNREPRESENTABLE_FLOAT_VALUE:
            return "UNREPRESENTABLE_FLOAT_VALUE";
        case Error::UNKNOWN_ENUM_VALUE:
            return "UNKNOWN_ENUM_VALUE";
    }
    return "";
}

AnimationSerializer::AnimationSerializer(std::string &json) : json(json) {
    json.clear();
}

void AnimationSerializer::writeEscaped(char c) {
    switch (c) {
        case '\x00': json += "\\u0000"; break;
        case '\x01': json += "\\u0001"; break;
        case '\x02': json += "\\u0002"; break;
        case '\x03': json += "\\u0003"; break;
        case '\x04': json += "\\u0004"; break;
        case '\x05': json += "\\u0005"; break;
        case '\x06': json += "\\u0006"; break;
        case '\x07': json += "\\u0007"; break;
        case '\b': json += "\\b"; break;
        case '\t': json += "\\t"; break;
        case '\n': json += "\\n"; break;
        case '\x0b': json += "\\u000b"; break;
        case '\f': json += "\\f"; break;
        case '\r': json += "\\r"; break;
        case '\x0e': json += "\\u000e"; break;
        case '\x0f': json += "\\u000f"; break;
        case '\x10': json += "\\u0010"; break;
        case '\x11': json += "\\u0011"; break;
        case '\x12': json += "\\u0012"; break;
        case '\x13': json += "\\u0013"; break;
        case '\x14': json += "\\u0014"; break;
        case '\x15': json += "\\u0015"; break;
        case '\x16': json += "\\u0016"; break;
        case '\x17': json += "\\u0017"; break;
        case '\x18': json += "\\u0018"; break;
        case '\x19': json += "\\u0019"; break;
        case '\x1a': json += "\\u001a"; break;
        case '\x1b': json += "\\u001b"; break;
        case '\x1c': json += "\\u001c"; break;
        case '\x1d': json += "\\u001d"; break;
        case '\x1e': json += "\\u001e"; break;
        case '\x1f': json += "\\u001f"; break;
        case '"': json += "\\\""; break;
        case '\\': json += "\\\\"; break;
        default:
            json.push_back(c);
    }
}

AnimationSerializer::Error AnimationSerializer::serialize(std::string &jsonString, const ode::DocumentAnimation &input) {
    return AnimationSerializer(jsonString).serializeOdeDocumentAnimation(input);
}

AnimationSerializer::Error AnimationSerializer::serializeStdString(const std::string &value) {
    json.push_back('"');
    for (std::string::const_iterator i = value.begin(), end = value.end(); i != end; ++i) { char c = *i; writeEscaped(c); }
    json.push_back('"');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeOdeLayerAnimationType(const ode::LayerAnimation::Type &value) {
    switch (value) {
        case ode::LayerAnimation::TRANSFORM: json += "\"TRANSFORM\""; break;
        case ode::LayerAnimation::ROTATION: json += "\"ROTATION\""; break;
        case ode::LayerAnimation::OPACITY: json += "\"OPACITY\""; break;
        case ode::LayerAnimation::FILL_COLOR: json += "\"FILL_COLOR\""; break;
        default:
            return Error(Error::UNKNOWN_ENUM_VALUE, &value);
    }
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeDouble(const double &value) {
    char buffer[64];
    JSON_CPP_SERIALIZE_DOUBLE(buffer, value);
    switch (buffer[1]) {
        case 'i':
            json += "-1e999";
            break;
        case 'n':
            if (buffer[0] == 'i') {
            json += "1e999";
                break;
            }
            // fallthrough
        case 'a':
            return Error(Error::UNREPRESENTABLE_FLOAT_VALUE, &value);
            break;
        default:
            json += buffer;
    }
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeStdVectorDouble(const std::vector<double> &value) {
    bool prev = false;
    json.push_back('[');
    for (std::vector<double>::const_iterator i = value.begin(), end = value.end(); i != end; ++i) { double const &elem = *i; if (prev) { json.push_back(','); } prev = true; if (Error error = serializeDouble(elem)) return error; }
    json.push_back(']');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeStdArrayDouble6(const std::array<double, 6> &value) {
    json.push_back('[');
    if (Error error = serializeDouble(value[0]))
        return error;
    for (int i = 1; i < 6; ++i) {
        json.push_back(',');
        if (Error error = serializeDouble(value[i]))
            return error;
    }
    json.push_back(']');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeOctopusColor(const octopus::Color &value) {
    json += "{\"" "r" "\":";
    if (Error error = serializeDouble(value.r))
        return error;
    json += ",\"" "g" "\":";
    if (Error error = serializeDouble(value.g))
        return error;
    json += ",\"" "b" "\":";
    if (Error error = serializeDouble(value.b))
        return error;
    json += ",\"" "a" "\":";
    if (Error error = serializeDouble(value.a))
        return error;
    json.push_back('}');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeOdeLayerAnimationKeyframe(const ode::LayerAnimation::Keyframe &value) {
    json += "{\"" "delay" "\":";
    if (Error error = serializeDouble(value.delay))
        return error;
    if (value.easing.has_value()) {
        json += ",\"" "easing" "\":";
        if (Error error = serializeStdVectorDouble(value.easing.value()))
            return error;
    }
    if (value.transform.has_value()) {
        json += ",\"" "transform" "\":";
        if (Error error = serializeStdArrayDouble6(value.transform.value()))
            return error;
    }
    if (value.rotation.has_value()) {
        json += ",\"" "rotation" "\":";
        if (Error error = serializeDouble(value.rotation.value()))
            return error;
    }
    if (value.opacity.has_value()) {
        json += ",\"" "opacity" "\":";
        if (Error error = serializeDouble(value.opacity.value()))
            return error;
    }
    if (value.color.has_value()) {
        json += ",\"" "color" "\":";
        if (Error error = serializeOctopusColor(value.color.value()))
            return error;
    }
    json.push_back('}');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeStdVectorOdeLayerAnimationKeyframe(const std::vector<ode::LayerAnimation::Keyframe> &value) {
    bool prev = false;
    json.push_back('[');
    for (std::vector<ode::LayerAnimation::Keyframe>::const_iterator i = value.begin(), end = value.end(); i != end; ++i) { ode::LayerAnimation::Keyframe const &elem = *i; if (prev) { json.push_back(','); } prev = true; if (Error error = serializeOdeLayerAnimationKeyframe(elem)) return error; }
    json.push_back(']');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeStdArrayDouble2(const std::array<double, 2> &value) {
    json.push_back('[');
    if (Error error = serializeDouble(value[0]))
        return error;
    for (int i = 1; i < 2; ++i) {
        json.push_back(',');
        if (Error error = serializeDouble(value[i]))
            return error;
    }
    json.push_back(']');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeOdeLayerAnimation(const ode::LayerAnimation &value) {
    json += "{\"" "layer" "\":";
    if (Error error = serializeStdString(value.layer))
        return error;
    json += ",\"" "type" "\":";
    if (Error error = serializeOdeLayerAnimationType(value.type))
        return error;
    json += ",\"" "keyframes" "\":";
    if (Error error = serializeStdVectorOdeLayerAnimationKeyframe(value.keyframes))
        return error;
    if (value.rotationCenter.has_value()) {
        json += ",\"" "rotationCenter" "\":";
        if (Error error = serializeStdArrayDouble2(value.rotationCenter.value()))
            return error;
    }
    json.push_back('}');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeStdVectorOdeLayerAnimation(const std::vector<ode::LayerAnimation> &value) {
    bool prev = false;
    json.push_back('[');
    for (std::vector<ode::LayerAnimation>::const_iterator i = value.begin(), end = value.end(); i != end; ++i) { ode::LayerAnimation const &elem = *i; if (prev) { json.push_back(','); } prev = true; if (Error error = serializeOdeLayerAnimation(elem)) return error; }
    json.push_back(']');
    return Error::OK;
}

AnimationSerializer::Error AnimationSerializer::serializeOdeDocumentAnimation(const ode::DocumentAnimation &value) {
    json += "{\"" "animations" "\":";
    if (Error error = serializeStdVectorOdeLayerAnimation(value.animations))
        return error;
    json.push_back('}');
    return Error::OK;
}

}
