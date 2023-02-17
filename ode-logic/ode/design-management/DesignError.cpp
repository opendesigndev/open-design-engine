
#include "DesignError.h"

namespace ode {

DesignError::DesignError(Error error) : error(error) { }

DesignError::DesignError(octopus::Parser::Error octopusParseError) : error(octopusParseError ? OCTOPUS_PARSE_ERROR : OK) { }

DesignError::DesignError(octopus::Serializer::Error octopusSerializationError) : error(octopusSerializationError ? OCTOPUS_UNAVAILABLE : OK) { }

DesignError::DesignError(octopus::ManifestParser::Error manifestParseError) : error(manifestParseError ? OCTOPUS_MANIFEST_PARSE_ERROR : OK) { }

DesignError::DesignError(octopus::ManifestSerializer::Error manifestSerializationError) : error(manifestSerializationError ? UNKNOWN_ERROR : OK) { } // TODO

DesignError::Error DesignError::type() const {
    return error;
}

DesignError::operator bool() const {
    return error != OK;
}

}
