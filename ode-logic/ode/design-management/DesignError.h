
#pragma once

#include <octopus/parser.h>
#include <octopus/serializer.h>
#include <octopus-manifest/parser.h>
#include <octopus-manifest/serializer.h>

namespace ode {

class DesignError {

public:
    enum Error {
        OK = 0,
        UNKNOWN_ERROR,
        FILE_READ_ERROR,
        FILE_WRITE_ERROR,
        OCTOPUS_PARSE_ERROR,
        OCTOPUS_MANIFEST_PARSE_ERROR,
        ANIMATION_PARSE_ERROR,
        ITEM_NOT_FOUND,
        LAYER_NOT_FOUND,
        COMPONENT_NOT_FOUND,
        DUPLICATE_COMPONENT_ID,
        DUPLICATE_LAYER_ID,
        OCTOPUS_UNAVAILABLE,
        COMPONENT_IN_USE,
        ALREADY_INITIALIZED,
        SHAPE_LAYER_ERROR,
        TEXT_LAYER_ERROR,
        WRONG_LAYER_TYPE,
        LAYER_CHANGE_INVALID_OP,
        LAYER_CHANGE_INVALID_SUBJECT,
        LAYER_CHANGE_INVALID_INDEX,
        LAYER_CHANGE_MISSING_VALUE,
        SINGULAR_TRANSFORMATION,
        INVALID_DESIGN,
        INVALID_COMPONENT,
        NOT_IMPLEMENTED
    };

    DesignError(Error error = OK);
    DesignError(octopus::Parser::Error octopusParseError);
    DesignError(octopus::Serializer::Error octopusSerializationError);
    DesignError(octopus::ManifestParser::Error manifestParseError);
    DesignError(octopus::ManifestSerializer::Error manifestSerializationError);
    Error type() const;
    explicit operator bool() const;

private:
    Error error;

};

}
