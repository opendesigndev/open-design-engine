
#include "logic-api.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <octopus/octopus.h>
#include <octopus-manifest/octopus-manifest.h>
#include <octopus/parser.h>
#include <octopus-manifest/parser.h>
#include <ode-essentials.h>
#include "animation/DocumentAnimation.h"
#include "animation/AnimationParser.h"
#include "design-management/Design.h"

using namespace ode;

struct ODE_internal_Engine {
    std::vector<void *> fontDataBuffers;
};

struct ODE_internal_Design {
    ODE_internal_Engine *engine;
    Design design;

    inline ODE_internal_Design(ODE_internal_Engine *engine) : engine(engine), design() { }
    inline explicit ODE_internal_Design(ODE_internal_Engine *engine, Design &&design) : engine(engine), design((Design &&) design) { }
};

struct ODE_internal_Component {
    Design::ComponentAccessor accessor;

    inline ODE_internal_Component() : accessor() { }
    inline explicit ODE_internal_Component(const Design::ComponentAccessor &accessor) : accessor(accessor) { }
};

// Implementation - TODO move

ODE_Result ode_result(DesignError::Error error) {
    switch (error) {
        case DesignError::OK:
            return ODE_RESULT_OK;
        case DesignError::UNKNOWN_ERROR:
            return ODE_RESULT_UNKNOWN_ERROR;
        case DesignError::FILE_READ_ERROR:
            return ODE_RESULT_FILE_READ_ERROR;
        case DesignError::FILE_WRITE_ERROR:
            return ODE_RESULT_FILE_WRITE_ERROR;
        case DesignError::OCTOPUS_PARSE_ERROR:
            return ODE_RESULT_OCTOPUS_PARSE_ERROR;
        case DesignError::OCTOPUS_MANIFEST_PARSE_ERROR:
            return ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR;
        case DesignError::ANIMATION_PARSE_ERROR:
            return ODE_RESULT_ANIMATION_PARSE_ERROR;
        case DesignError::ITEM_NOT_FOUND:
            return ODE_RESULT_ITEM_NOT_FOUND;
        case DesignError::LAYER_NOT_FOUND:
            return ODE_RESULT_LAYER_NOT_FOUND;
        case DesignError::COMPONENT_NOT_FOUND:
            return ODE_RESULT_COMPONENT_NOT_FOUND;
        case DesignError::DUPLICATE_COMPONENT_ID:
            return ODE_RESULT_DUPLICATE_COMPONENT_ID;
        case DesignError::DUPLICATE_LAYER_ID:
            return ODE_RESULT_DUPLICATE_LAYER_ID;
        case DesignError::OCTOPUS_UNAVAILABLE:
            return ODE_RESULT_OCTOPUS_UNAVAILABLE;
        case DesignError::COMPONENT_IN_USE:
            return ODE_RESULT_COMPONENT_IN_USE;
        case DesignError::ALREADY_INITIALIZED:
            return ODE_RESULT_ALREADY_INITIALIZED;
        case DesignError::SHAPE_LAYER_ERROR:
            return ODE_RESULT_SHAPE_LAYER_ERROR;
        case DesignError::TEXT_LAYER_ERROR:
            return ODE_RESULT_TEXT_LAYER_ERROR;
        case DesignError::WRONG_LAYER_TYPE:
            return ODE_RESULT_WRONG_LAYER_TYPE;
        case DesignError::INVALID_DESIGN:
            return ODE_RESULT_INVALID_DESIGN;
        case DesignError::INVALID_COMPONENT:
            return ODE_RESULT_INVALID_COMPONENT;
        case DesignError::NOT_IMPLEMENTED:
            return ODE_RESULT_NOT_IMPLEMENTED;
    }
    ODE_ASSERT(!"switch incomplete");
    return ODE_RESULT_UNKNOWN_ERROR;
}

ODE_LayerType ode_layerType(octopus::Layer::Type type) {
    switch (type) {
        case octopus::Layer::Type::SHAPE:
            return ODE_LAYER_TYPE_SHAPE;
        case octopus::Layer::Type::TEXT:
            return ODE_LAYER_TYPE_TEXT;
        case octopus::Layer::Type::GROUP:
            return ODE_LAYER_TYPE_GROUP;
        case octopus::Layer::Type::MASK_GROUP:
            return ODE_LAYER_TYPE_MASK_GROUP;
        case octopus::Layer::Type::COMPONENT_REFERENCE:
            return ODE_LAYER_TYPE_COMPONENT_REFERENCE;
        case octopus::Layer::Type::COMPONENT_INSTANCE:
            return ODE_LAYER_TYPE_COMPONENT_INSTANCE;
    }
    ODE_ASSERT(!"switch incomplete");
    return ODE_LAYER_TYPE_UNSPECIFIED;
}

template <typename ErrorType>
static ODE_ParseError::Type ode_parseErrorType(ErrorType type) {
    switch (type) {
        case ErrorType::OK:
            return ODE_ParseError::OK;
        case ErrorType::JSON_SYNTAX_ERROR:
            return ODE_ParseError::JSON_SYNTAX_ERROR;
        case ErrorType::UNEXPECTED_END_OF_FILE:
            return ODE_ParseError::UNEXPECTED_END_OF_FILE;
        case ErrorType::TYPE_MISMATCH:
            return ODE_ParseError::TYPE_MISMATCH;
        case ErrorType::ARRAY_SIZE_MISMATCH:
            return ODE_ParseError::ARRAY_SIZE_MISMATCH;
        case ErrorType::UNKNOWN_KEY:
            return ODE_ParseError::UNKNOWN_KEY;
        case ErrorType::UNKNOWN_ENUM_VALUE:
            return ODE_ParseError::UNKNOWN_ENUM_VALUE;
        case ErrorType::VALUE_OUT_OF_RANGE:
            return ODE_ParseError::VALUE_OUT_OF_RANGE;
        case ErrorType::STRING_EXPECTED:
            return ODE_ParseError::STRING_EXPECTED;
        case ErrorType::UTF16_ENCODING_ERROR:
            return ODE_ParseError::UTF16_ENCODING_ERROR;
    }
    ODE_ASSERT(!"switch incomplete");
    return ODE_ParseError::JSON_SYNTAX_ERROR;
}

static void gatherLayerList(std::vector<ODE_LayerList::Entry> &list, const octopus::Layer &layer, const ODE_StringRef &parentId, int flags) {
    ODE_LayerList::Entry entry;
    entry.parentId = parentId;
    entry.id = ode_stringRef(layer.id);
    entry.type = ode_layerType(layer.type);
    entry.flags = flags|(layer.visible ? ODE_LAYER_FLAG_VISIBLE : 0);
    entry.name = ode_stringRef(layer.name);
    list.push_back(entry);
    if (layer.type == octopus::Layer::Type::MASK_GROUP && layer.mask.has_value())
        gatherLayerList(list, *layer.mask, entry.id, ODE_LAYER_FLAG_MASK);
    if (layer.layers.has_value() && (layer.type == octopus::Layer::Type::GROUP || layer.type == octopus::Layer::Type::MASK_GROUP || layer.type == octopus::Layer::Type::COMPONENT_INSTANCE)) {
        for (const octopus::Layer &child : layer.layers.value())
            gatherLayerList(list, child, entry.id, 0);
    }
}

static ODE_Result makeFontList(ODE_StringList *fontList, const std::set<std::string> &fontSet) {
    ODE_ASSERT(fontList);
    fontList->entries = nullptr;
    fontList->n = 0;
    if (!fontSet.empty()) {
        if (!(fontList->entries = reinterpret_cast<ODE_StringRef *>(malloc(sizeof(ODE_StringRef)*fontSet.size()))))
            return ODE_RESULT_MEMORY_ALLOCATION_ERROR;
        size_t totalStrLen = 0;
        for (const std::string &font : fontSet)
            totalStrLen += font.size()+1;
        char *cur = nullptr;
        if (!(cur = reinterpret_cast<char *>(malloc(totalStrLen)))) {
            free(fontList->entries);
            fontList->entries = nullptr;
            return ODE_RESULT_MEMORY_ALLOCATION_ERROR;
        }
        for (const std::string &font : fontSet) {
            fontList->entries[fontList->n].data = reinterpret_cast<ODE_ConstCharPtr>(cur);
            memcpy(cur, font.c_str(), font.size()+1);
            fontList->entries[fontList->n].length = int(font.size());
            cur += font.size()+1;
            ++fontList->n;
        }
        ODE_ASSERT(fontList->n == fontSet.size());
        ODE_ASSERT(cur == fontList->entries->data+totalStrLen);
    }
    return ODE_RESULT_OK;
}

// General

ODE_Result ODE_API ode_destroyLayerList(ODE_LayerList layerList) {
    free(layerList.entries);
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_destroyMissingFontList(ODE_StringList fontList) {
    if (fontList.n) {
        ODE_ASSERT(fontList.entries);
        // Listing functions guarantee that all strings in the list are in a single memory block that starts at the first string
        free(const_cast<char *>(reinterpret_cast<const char *>(fontList.entries->data)));
        free(fontList.entries);
    }
    return ODE_RESULT_OK;
}

// Engine

ODE_Result ODE_API ode_initializeEngineAttributes(ODE_EngineAttributes *engineAttributes) {
    ODE_ASSERT(engineAttributes);
    engineAttributes->padding = 0;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_createEngine(ODE_EngineHandle *engine, const ODE_EngineAttributes *engineAttributes) {
    ODE_ASSERT(engine);
    engine->ptr = new ODE_internal_Engine;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_destroyEngine(ODE_EngineHandle engine) {
    if (engine.ptr) {
        for (void *dataPtr : engine.ptr->fontDataBuffers)
            free(dataPtr);
        // TODO check that all designs destroyed
        delete engine.ptr;
    }
    return ODE_RESULT_OK;
}

// Design

ODE_Result ODE_API ode_createDesign(ODE_EngineHandle engine, ODE_DesignHandle *design) {
    ODE_ASSERT(engine.ptr && design);
    design->ptr = new ODE_internal_Design(engine.ptr);
    return ODE_RESULT_OK;
}

// Design file format does not exist yet
//ODE_Result ODE_NATIVE_API ode_loadDesignFromFile(ODE_EngineHandle engine, ODE_DesignHandle *design, ODE_StringRef path, ODE_ParseError *parseError);

ODE_Result ODE_NATIVE_API ode_loadDesignFromManifestFile(ODE_EngineHandle engine, ODE_DesignHandle *design, ODE_StringRef path, ODE_ParseError *parseError) {
    ODE_ASSERT(design && path.data);
    std::string manifestString;
    if (!readFile(ode_stringDeref(path), manifestString))
        return ODE_RESULT_FILE_READ_ERROR;
    return ode_loadDesignFromManifestString(engine, design, ode_stringRef(manifestString), parseError);
}

ODE_Result ODE_API ode_loadDesignFromManifestString(ODE_EngineHandle engine, ODE_DesignHandle *design, ODE_StringRef manifestString, ODE_ParseError *parseError) {
    ODE_ASSERT(engine.ptr && design && manifestString.data);
    ODE_ASSERT(!manifestString.data[manifestString.length]);
    octopus::OctopusManifest manifest;
    if (octopus::ManifestParser::Error error = octopus::ManifestParser::parse(manifest, reinterpret_cast<const char *>(manifestString.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR;
    }
    if (Result<Design, DesignError> result = Design::loadManifest(manifest)) {
        design->ptr = new ODE_internal_Design(engine.ptr, (Design &&) result.value());
        return ODE_RESULT_OK;
    } else
        return ode_result(result.error().type());
}

ODE_Result ODE_API ode_destroyDesign(ODE_DesignHandle design) {
    delete design.ptr;
    return ODE_RESULT_OK;
}

ODE_Result ODE_NATIVE_API ode_design_loadManifestFile(ODE_DesignHandle design, ODE_StringRef path, ODE_ParseError *parseError) {
    ODE_ASSERT(path.data);
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    std::string manifestString;
    if (!readFile(ode_stringDeref(path), manifestString))
        return ODE_RESULT_FILE_READ_ERROR;
    return ode_design_loadManifestString(design, ode_stringRef(manifestString), parseError);
}

ODE_Result ODE_API ode_design_loadManifestString(ODE_DesignHandle design, ODE_StringRef manifestString, ODE_ParseError *parseError) {
    ODE_ASSERT(manifestString.data);
    ODE_ASSERT(!manifestString.data[manifestString.length]);
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    octopus::OctopusManifest manifest;
    if (octopus::ManifestParser::Error error = octopus::ManifestParser::parse(manifest, reinterpret_cast<const char *>(manifestString.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR;
    }
    if (Result<Design, DesignError> result = design.ptr->design.loadManifest(manifest))
        return ODE_RESULT_OK;
    else
        return ode_result(result.error().type());
}

// TODO
//ODE_Result ODE_API ode_design_addComponentFromManifestString(ODE_DesignHandle design, ODE_ComponentHandle *component, ODE_StringRef componentManifestString, ODE_ParseError *parseError);

ODE_Result ODE_API ode_design_addComponentFromOctopusString(ODE_DesignHandle design, ODE_ComponentHandle *component, ODE_ComponentMetadata metadata, ODE_StringRef octopusString, ODE_ParseError *parseError) {
    ODE_ASSERT(component && octopusString.data);
    ODE_ASSERT(!octopusString.data[octopusString.length]);
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    octopus::Octopus octopus;
    if (octopus::Parser::Error error = octopus::Parser::parse(octopus, reinterpret_cast<const char *>(octopusString.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_PARSE_ERROR;
    }
    octopus::Component manifest;
    if (metadata.id.data)
        manifest.id = ode_stringDeref(metadata.id);
    else
        manifest.id = octopus.id;
    manifest.bounds.x = metadata.position.x;
    manifest.bounds.y = metadata.position.y;
    if (DesignError error = design.ptr->design.addComponent(manifest, (octopus::Octopus &&) octopus))
        return ode_result(error.type());
    component->ptr = new ODE_internal_Component(design.ptr->design.getComponent(manifest.id));
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_design_removeComponent(ODE_DesignHandle design, ODE_ComponentHandle component) {
    ODE_ASSERT(component.ptr);
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    return ode_result(design.ptr->design.removeComponent(component.ptr->accessor).type());
}

ODE_Result ODE_API ode_design_listMissingFonts(ODE_DesignHandle design, ODE_StringList *fontList) {
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    std::set<std::string> missingFonts;
    design.ptr->design.listMissingFonts(missingFonts);
    return makeFontList(fontList, missingFonts);
}

ODE_Result ODE_NATIVE_API ode_design_loadFontFile(ODE_DesignHandle design, ODE_StringRef name, ODE_StringRef path, ODE_StringRef faceName) {
    if (!textify::addFontFile(TEXTIFY_CONTEXT, ode_stringDeref(name), ode_stringDeref(faceName), ode_stringDeref(path), false))
        return ODE_RESULT_FONT_ERROR;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_design_loadFontBytes(ODE_DesignHandle design, ODE_StringRef name, ODE_MemoryBuffer *data, ODE_StringRef faceName) {
    ODE_ASSERT(data);
    if (!textify::addFontBytes(TEXTIFY_CONTEXT, ode_stringDeref(name), ode_stringDeref(faceName), reinterpret_cast<const uint8_t *>(data->data), data->length, false))
        return ODE_RESULT_FONT_ERROR;
    // Transfer data pointer from memory buffer to engine
    design.ptr->engine->fontDataBuffers.push_back(reinterpret_cast<void *>(data->data));
    data->data = ODE_VarDataPtr();
    data->length = 0;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_design_getComponent(ODE_DesignHandle design, ODE_ComponentHandle *component, ODE_StringRef componentId) {
    ODE_ASSERT(component && componentId.data);
    if (!design.ptr)
        return ODE_RESULT_INVALID_DESIGN;
    if (Design::ComponentAccessor accessor = design.ptr->design.getComponent(ode_stringDeref(componentId))) {
        component->ptr = new ODE_internal_Component(accessor);
        return ODE_RESULT_OK;
    }
    return ODE_RESULT_COMPONENT_NOT_FOUND;
}

// Component

// TODO
//ODE_Result ODE_NATIVE_API ode_component_loadOctopusFile(ODE_ComponentHandle component, ODE_StringRef path, ODE_StringRef assetBasePath, ODE_ParseError *parseError);
//ODE_Result ODE_API ode_component_loadOctopusString(ODE_ComponentHandle component, ODE_StringRef octopusString, ODE_StringRef assetBasePath, ODE_ParseError *parseError);
//ODE_Result ODE_API ode_component_setRootLayer(ODE_ComponentHandle component, ODE_StringRef layerOctopusString, ODE_ParseError *parseError);

ODE_Result ODE_API ode_component_addLayer(ODE_ComponentHandle component, ODE_StringRef parentLayerId, ODE_StringRef beforeLayerId, ODE_StringRef layerOctopusString, ODE_ParseError *parseError) {
    ODE_ASSERT(parentLayerId.data && layerOctopusString.data);
    ODE_ASSERT(!layerOctopusString.data[layerOctopusString.length]);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    octopus::Layer layer;
    if (octopus::Parser::Error error = octopus::Parser::parse(layer, reinterpret_cast<const char *>(layerOctopusString.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_PARSE_ERROR;
    }
    if (DesignError error = component.ptr->accessor.addLayer(ode_stringDeref(parentLayerId), ode_stringDeref(beforeLayerId), layer))
        return ode_result(error.type());
    return ODE_RESULT_OK;
}

//ODE_Result ODE_API ode_component_removeLayer(ODE_ComponentHandle component, ODE_StringRef layerId);

ODE_Result ODE_API ode_component_modifyLayer(ODE_ComponentHandle component, ODE_StringRef layerId, ODE_StringRef layerChangeOctopusString, ODE_ParseError *parseError) {
    ODE_ASSERT(layerId.data && layerChangeOctopusString.data);
    ODE_ASSERT(!layerChangeOctopusString.data[layerChangeOctopusString.length]);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    octopus::LayerChange change;
    if (octopus::Parser::Error error = octopus::Parser::parse(change, reinterpret_cast<const char *>(layerChangeOctopusString.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_OCTOPUS_PARSE_ERROR;
    }
    return ode_result(component.ptr->accessor.modifyLayer(ode_stringDeref(layerId), change).type());
}

ODE_Result ODE_API ode_pr1_component_loadAnimation(ODE_ComponentHandle component, ODE_StringRef animationDefinition, ODE_ParseError *parseError) {
    ODE_ASSERT(animationDefinition.data);
    ODE_ASSERT(!animationDefinition.data[animationDefinition.length]);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    DocumentAnimation animation;
    if (AnimationParser::Error error = AnimationParser::parse(animation, reinterpret_cast<const char *>(animationDefinition.data))) {
        if (parseError) {
            parseError->type = ode_parseErrorType(error.type);
            parseError->position = error.position;
        }
        return ODE_RESULT_ANIMATION_PARSE_ERROR;
    }
    if (DesignError error = component.ptr->accessor.setAnimation(animation))
        return ode_result(error.type());
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_pr1_component_getAnimationValueAtTime(ODE_ComponentHandle component, int index, ODE_Scalar time, ODE_VarDataPtr value) {
    ODE_ASSERT(value);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    if (Result<LayerAnimation::Keyframe, DesignError> result = component.ptr->accessor.getAnimationValue(index, time)) {
        const LayerAnimation::Keyframe &frame = result.value();
        ODE_Scalar *dst = reinterpret_cast<ODE_Scalar *>(value);
        if (frame.transform.has_value()) {
            static_assert(sizeof(*dst) == sizeof(*frame.transform.value().data()), "Assuming ODE_Scalar is double");
            memcpy(dst, frame.transform.value().data(), 6*sizeof(ODE_Scalar));
        } else if (frame.rotation.has_value())
            *dst = frame.rotation.value();
        else if (frame.opacity.has_value())
            *dst = frame.opacity.value();
        else if (frame.color.has_value()) {
            dst[0] = frame.color->r;
            dst[1] = frame.color->g;
            dst[2] = frame.color->b;
            dst[3] = frame.color->a;
        } else {
            ODE_ASSERT(!"Missing implementation?");
            return ODE_RESULT_UNKNOWN_ERROR;
        }
        return ODE_RESULT_OK;
    } else
        return ode_result(result.error().type());
}

ODE_Result ODE_API ode_component_listLayers(ODE_ComponentHandle component, ODE_LayerList *layerList) {
    ODE_ASSERT(layerList);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    if (component.ptr->accessor.getOctopus().content.has_value()) {
        std::vector<ODE_LayerList::Entry> list;
        gatherLayerList(list, *component.ptr->accessor.getOctopus().content, ODE_StringRef(), 0);
        if (!(layerList->entries = reinterpret_cast<ODE_LayerList::Entry *>(malloc(sizeof(ODE_LayerList::Entry)*list.size()))))
            return ODE_RESULT_MEMORY_ALLOCATION_ERROR;
        memcpy(layerList->entries, list.data(), sizeof(ODE_LayerList::Entry)*list.size());
        layerList->n = list.size();
    } else {
        layerList->entries = nullptr;
        layerList->n = 0;
    }
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_component_identifyLayer(ODE_ComponentHandle component, ODE_String *layerId, ODE_Vector2 position, ODE_Scalar radius) {
    ODE_ASSERT(layerId);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    std::string id = component.ptr->accessor.identifyLayer(Vector2d(position.x, position.y), radius);
    if (id.empty()) {
        layerId->data = nullptr;
        layerId->length = 0;
    } else
        *layerId = ode_makeString(id);
    return ODE_RESULT_OK;
}

template <int VARIANT>
static ODE_Rectangle ode_rect(const Rectangle<double, VARIANT> &rect) {
    ODE_Rectangle result;
    result.a.x = rect.a.x;
    result.a.y = rect.a.y;
    result.b.x = rect.b.x;
    result.b.y = rect.b.y;
    return result;
}

ODE_Result ODE_API ode_component_getLayerMetrics(ODE_ComponentHandle component, ODE_StringRef layerId, ODE_LayerMetrics *layerMetrics) {
    ODE_ASSERT(layerId.data && layerMetrics);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    if (Result<LayerMetrics, DesignError> metrics = component.ptr->accessor.getLayerMetrics(ode_stringDeref(layerId))) {
        layerMetrics->logicalBounds = ode_rect(metrics.value().logicalBounds);
        layerMetrics->graphicalBounds = ode_rect(metrics.value().untransformedBounds);
        layerMetrics->transformedGraphicalBounds = ode_rect(metrics.value().bounds);
        layerMetrics->transformation.matrix[0] = metrics.value().transformation[0][0];
        layerMetrics->transformation.matrix[1] = metrics.value().transformation[0][1];
        layerMetrics->transformation.matrix[2] = metrics.value().transformation[1][0];
        layerMetrics->transformation.matrix[3] = metrics.value().transformation[1][1];
        layerMetrics->transformation.matrix[4] = metrics.value().transformation[2][0];
        layerMetrics->transformation.matrix[5] = metrics.value().transformation[2][1];
        return ODE_RESULT_OK;
    } else
        return ode_result(metrics.error().type());
}

ODE_Result ODE_API ode_component_listMissingFonts(ODE_ComponentHandle component, ODE_StringList *fontList) {
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    std::set<std::string> missingFonts;
    component.ptr->accessor.listMissingFonts(missingFonts);
    return makeFontList(fontList, missingFonts);
}

ODE_Result ODE_API ode_component_getOctopus(ODE_ComponentHandle component, ODE_String *octopusString) {
    ODE_ASSERT(octopusString);
    if (!component.ptr)
        return ODE_RESULT_INVALID_COMPONENT;
    std::string json;
    if (octopus::Serializer::Error error = octopus::Serializer::serialize(json, component.ptr->accessor.getOctopus()))
        return ODE_RESULT_OCTOPUS_PARSE_ERROR;
    *octopusString = ode_makeString((std::string &&) json);
    return ODE_RESULT_OK;
}

// TODO
//ODE_Result ODE_API ode_component_getInstancedOctopus(ODE_ComponentHandle component, ODE_String *octopusString);
//ODE_Result ODE_API ode_component_getLayerOctopus(ODE_ComponentHandle *component, ODE_String *octopusString, ODE_StringRef layerId);

/*int ODE_API ode_createDocument(ODE_Component **component) {
    ODE_ASSERT(component);
    *component = new ODE_ComponentData(ComponentPtr(new Component));
    return 0;
}

int ODE_API ode_loadDocumentFromOctopusFile(ODE_Component **component, const char *path, const char *assetBasePath) {
    ODE_ASSERT(component && path);
    std::string octopusString;
    if (!readFile(path, octopusString))
        return int(DesignError::FILE_READ_ERROR);
    return ode_loadDocumentFromOctopusString(component, octopusString.c_str(), assetBasePath);
}

int ODE_API ode_loadDocumentFromOctopusString(ODE_Component **component, const char *octopusString, const char *assetBasePath) {
    ODE_ASSERT(component && octopusString);
    octopus::Octopus octopus;
    if (octopus::Parser::Error error = octopus::Parser::parse(octopus, octopusString))
        return int(DesignError::OCTOPUS_PARSE_ERROR);
    ComponentPtr componentPtr(new Component);
    // TODO init assetBase
    if (DesignError error = componentPtr->initialize((octopus::Octopus &&) octopus))
        return int(error.type());
    *component = new ODE_ComponentData((ComponentPtr &&) componentPtr);
    return 0;
}

int ODE_API ode_destroyDocument(ODE_Component *component) {
    ODE_ASSERT(!component || component->ownedComponent);
    delete component;
    return 0;
}

int ODE_API ode_component_loadOctopusFile(ODE_Component *component, const char *path, const char *assetBasePath) {
    ODE_ASSERT(path);
    if (!component)
        return int(DesignError::INVALID_COMPONENT);
    if (!component->ownedComponent) // Operation only permitted on standalone documents
        return int(DesignError::ALREADY_INITIALIZED);
    std::string octopusString;
    if (!readFile(path, octopusString))
        return int(DesignError::FILE_READ_ERROR);
    octopus::Octopus octopus;
    if (octopus::Parser::Error error = octopus::Parser::parse(octopus, octopusString.c_str()))
        return int(DesignError::OCTOPUS_PARSE_ERROR);
    if (DesignError error = component->ownedComponent->initialize((octopus::Octopus &&) octopus))
        return int(error.type());
    return 0;
}

int ODE_API ode_component_loadOctopusString(ODE_Component *component, const char *octopusString, const char *assetBasePath) {
    ODE_ASSERT(octopusString);
    if (!component)
        return int(DesignError::INVALID_COMPONENT);
    if (!component->ownedComponent) // Operation only permitted on standalone documents
        return int(DesignError::ALREADY_INITIALIZED);
    octopus::Octopus octopus;
    if (octopus::Parser::Error error = octopus::Parser::parse(octopus, octopusString))
        return int(DesignError::OCTOPUS_PARSE_ERROR);
    if (DesignError error = component->ownedComponent->initialize((octopus::Octopus &&) octopus))
        return int(error.type());
    return 0;
}*/
