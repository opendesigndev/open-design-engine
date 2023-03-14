
// FILE GENERATED BY generate-api-bindings.py

#ifdef __EMSCRIPTEN__

#include <array>
#include <emscripten/bind.h>
#include "utils.h"
#include "api-base.h"

using namespace emscripten;

ODE_VarDataPtr ode_string_getData(const ODE_String &x) {
    return reinterpret_cast<ODE_VarDataPtr>(x.data);
}

const ODE_StringRef &ode_stringList_getEntry(const ODE_StringList &x, int i) {
    ODE_ASSERT(i >= 0 && i < x.n);
    return x.entries[i];
}

EMSCRIPTEN_BINDINGS(ode) {

    enum_<ODE_Result>("Result")
        .value("OK", ODE_RESULT_OK)
        .value("UNKNOWN_ERROR", ODE_RESULT_UNKNOWN_ERROR)
        .value("NOT_IMPLEMENTED", ODE_RESULT_NOT_IMPLEMENTED)
        .value("MEMORY_ALLOCATION_ERROR", ODE_RESULT_MEMORY_ALLOCATION_ERROR)
        .value("FILE_READ_ERROR", ODE_RESULT_FILE_READ_ERROR)
        .value("FILE_WRITE_ERROR", ODE_RESULT_FILE_WRITE_ERROR)
        .value("OCTOPUS_PARSE_ERROR", ODE_RESULT_OCTOPUS_PARSE_ERROR)
        .value("OCTOPUS_MANIFEST_PARSE_ERROR", ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR)
        .value("ANIMATION_PARSE_ERROR", ODE_RESULT_ANIMATION_PARSE_ERROR)
        .value("INVALID_ENUM_VALUE", ODE_RESULT_INVALID_ENUM_VALUE)
        .value("ITEM_NOT_FOUND", ODE_RESULT_ITEM_NOT_FOUND)
        .value("LAYER_NOT_FOUND", ODE_RESULT_LAYER_NOT_FOUND)
        .value("COMPONENT_NOT_FOUND", ODE_RESULT_COMPONENT_NOT_FOUND)
        .value("DUPLICATE_COMPONENT_ID", ODE_RESULT_DUPLICATE_COMPONENT_ID)
        .value("DUPLICATE_LAYER_ID", ODE_RESULT_DUPLICATE_LAYER_ID)
        .value("OCTOPUS_UNAVAILABLE", ODE_RESULT_OCTOPUS_UNAVAILABLE)
        .value("COMPONENT_IN_USE", ODE_RESULT_COMPONENT_IN_USE)
        .value("ALREADY_INITIALIZED", ODE_RESULT_ALREADY_INITIALIZED)
        .value("SHAPE_LAYER_ERROR", ODE_RESULT_SHAPE_LAYER_ERROR)
        .value("TEXT_LAYER_ERROR", ODE_RESULT_TEXT_LAYER_ERROR)
        .value("WRONG_LAYER_TYPE", ODE_RESULT_WRONG_LAYER_TYPE)
        .value("SINGULAR_TRANSFORMATION", ODE_RESULT_SINGULAR_TRANSFORMATION)
        .value("INVALID_DESIGN", ODE_RESULT_INVALID_DESIGN)
        .value("INVALID_COMPONENT", ODE_RESULT_INVALID_COMPONENT)
        .value("INVALID_PIXEL_FORMAT", ODE_RESULT_INVALID_PIXEL_FORMAT)
        .value("INVALID_BITMAP_DIMENSIONS", ODE_RESULT_INVALID_BITMAP_DIMENSIONS)
        .value("INVALID_RENDERER_CONTEXT", ODE_RESULT_INVALID_RENDERER_CONTEXT)
        .value("INVALID_IMAGE_BASE", ODE_RESULT_INVALID_IMAGE_BASE)
        .value("FONT_ERROR", ODE_RESULT_FONT_ERROR)
        .value("GRAPHICS_CONTEXT_ERROR", ODE_RESULT_GRAPHICS_CONTEXT_ERROR);

    value_array<ODE_Vector2>("Vector2")
        .element(&ODE_Vector2::x)
        .element(&ODE_Vector2::y);

    value_array<ODE_Rectangle>("Rectangle")
        .element(&ODE_Rectangle::a)
        .element(&ODE_Rectangle::b);

    value_object<ODE_StringRef>("StringRef")
        .field("data", &ODE_StringRef::data)
        .field("length", &ODE_StringRef::length);

    class_<ODE_String>("String").constructor<>(static_cast<ODE_String (*)(const std::string &)>(&ode_makeString))
        .property("length", &ODE_String::length)
        .function("ref", static_cast<ODE_StringRef (*)(const ODE_String &)>(&ode_stringRef))
        .function("getData", &ode_string_getData);

    class_<ODE_MemoryBuffer>("MemoryBuffer").constructor<>()
        .property("data", &ODE_MemoryBuffer::data)
        .property("length", &ODE_MemoryBuffer::length);

    class_<ODE_StringList>("StringList").constructor<>()
        .property("n", &ODE_StringList::n)
        .function("getEntry", &ode_stringList_getEntry);

    function("destroyString", &ode_destroyString, allow_raw_pointers());
    function("allocateMemoryBuffer", &ode_allocateMemoryBuffer, allow_raw_pointers());
    function("reallocateMemoryBuffer", &ode_reallocateMemoryBuffer, allow_raw_pointers());
    function("destroyMemoryBuffer", &ode_destroyMemoryBuffer, allow_raw_pointers());

}

#endif // __EMSCRIPTEN__
