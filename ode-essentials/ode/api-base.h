
#ifndef ODE_API_BASE_H
#define ODE_API_BASE_H

#include <stdlib.h>
#include <stdint.h>

#ifndef ODE_API
#define ODE_API // for DLL export etc.
#endif

/// Functions with ODE_NATIVE_API prefix are not available in Emscripten
#define ODE_NATIVE_API ODE_API
/// Functions with ODE_FUTURE_API prefix are not yet implemented
#define ODE_FUTURE_API ODE_API
/// Structures marked with ODE_TUPLE can be constructed as array objects in JS
#define ODE_TUPLE

// All pointer arguments must be const or marked with one of:
/// Pointer arguments marked with ODE_OUT will be written into but not read
#define ODE_OUT
/// Pointer arguments marked with ODE_INOUT will be read and written into
#define ODE_INOUT
/// Pointer argument marked with ODE_OUT_RETURN is output that can be considered the return value apart from Result
#define ODE_OUT_RETURN

#ifndef ODE_BIND_ARRAY_GETTER
#define ODE_BIND_ARRAY_GETTER(methodName, memberArray, memberCount)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Handle object declaration
#define ODE_HANDLE_DECL(T) struct T; typedef struct { struct T *ptr; }

/// Function call result type
typedef enum {
    ODE_RESULT_OK = 0,
    ODE_RESULT_UNKNOWN_ERROR = 1,
    ODE_RESULT_NOT_IMPLEMENTED = 2,
    ODE_RESULT_MEMORY_ALLOCATION_ERROR = 3,
    ODE_RESULT_FILE_READ_ERROR = 4,
    ODE_RESULT_FILE_WRITE_ERROR = 5,
    ODE_RESULT_OCTOPUS_PARSE_ERROR = 8,
    ODE_RESULT_OCTOPUS_MANIFEST_PARSE_ERROR = 9,
    ODE_RESULT_ANIMATION_PARSE_ERROR = 10,
    ODE_RESULT_INVALID_ENUM_VALUE = 15,
    ODE_RESULT_ITEM_NOT_FOUND = 16,
    ODE_RESULT_LAYER_NOT_FOUND = 17,
    ODE_RESULT_COMPONENT_NOT_FOUND = 18,
    ODE_RESULT_DUPLICATE_COMPONENT_ID = 20,
    ODE_RESULT_DUPLICATE_LAYER_ID = 21,
    ODE_RESULT_OCTOPUS_UNAVAILABLE = 22,
    ODE_RESULT_COMPONENT_IN_USE = 24,
    ODE_RESULT_ALREADY_INITIALIZED = 25,
    ODE_RESULT_SHAPE_LAYER_ERROR = 32,
    ODE_RESULT_TEXT_LAYER_ERROR = 33,
    ODE_RESULT_WRONG_LAYER_TYPE = 48,
    ODE_RESULT_SINGULAR_TRANSFORMATION = 49,
    ODE_RESULT_INVALID_DESIGN = 52,
    ODE_RESULT_INVALID_COMPONENT = 53,
    ODE_RESULT_INVALID_PIXEL_FORMAT = 96,
    ODE_RESULT_INVALID_BITMAP_DIMENSIONS = 97,
    ODE_RESULT_INVALID_RENDERER_CONTEXT = 108,
    ODE_RESULT_INVALID_IMAGE_BASE = 109,
    ODE_RESULT_FONT_ERROR = 119,
    ODE_RESULT_GRAPHICS_CONTEXT_ERROR = 128,
    ODE_RESULT_LAYER_CHANGE_INVALID_OP = 160,
    ODE_RESULT_LAYER_CHANGE_INVALID_SUBJECT = 161,
    ODE_RESULT_LAYER_CHANGE_INVALID_INDEX = 162,
    ODE_RESULT_LAYER_CHANGE_MISSING_VALUE = 163,
} ODE_Result;

/// A real floating-point value
typedef double ODE_Scalar;

// Raw data pointer types
typedef void *ODE_VarDataPtr;
typedef const void *ODE_ConstDataPtr;
typedef const char *ODE_ConstCharPtr;
typedef char *ODE_CharPtr;

/// A mathematical 2-dimensional vector
typedef struct { ODE_TUPLE
    ODE_Scalar x, y;
} ODE_Vector2;

/// An axis-aligned rectangle specified by its two opposite corners
typedef struct { ODE_TUPLE
    ODE_Vector2 a, b;
} ODE_Rectangle;

/// A reference to an immutable null-terminated string in contiguous memory (does not hold or change ownership)
typedef struct {
    /// Pointer to the beginning of UTF-8 encoded string
    ODE_ConstCharPtr data;
    /// Length of the string in bytes excluding the terminating null character
    int length;
} ODE_StringRef;

/// A standalone string in its own memory block (must be manually destroyed with ode_destroyString)
typedef struct {
    /// Pointer to the beginning of UTF-8 encoded string
    ODE_CharPtr data;
    /// Length of the string in bytes excluding the terminating null character
    int length;
} ODE_String;

/// A buffer of raw data bytes in physical memory - deallocate with ode_destroyMemoryBuffer
typedef struct {
    /// Pointer to the beginning of the memory block
    ODE_VarDataPtr data;
    /// Length of the buffer in bytes
    size_t length;
} ODE_MemoryBuffer;

/// A list of immutable string references
typedef struct {
    /// Array of strings
    ODE_StringRef *entries;
    /// Number of entries;
    int n;
    /// Get single entry
    ODE_BIND_ARRAY_GETTER(getEntry, entries, n);
} ODE_StringList;

/// Destroys the ODE_String object, freeing its allocated memory
ODE_Result ODE_API ode_destroyString(ODE_String string);

/**
 * Allocates a new memory buffer of a given size
 * @param buffer - the resulting memory buffer will be stored in this output argument
 * @param length - the desired length of the buffer in bytes
 */
ODE_Result ODE_API ode_allocateMemoryBuffer(ODE_OUT_RETURN ODE_MemoryBuffer *buffer, size_t length);

/**
 * Resizes an existing memory buffer to a given size, or allocates a new memory buffer if buffer's data and length are zero.
 * Pre-existing data in the buffer (up to the new length) will be preserved
 * @param buffer - the memory buffer to be resized
 * @param length - the desired new length of the buffer in bytes
 */
ODE_Result ODE_API ode_reallocateMemoryBuffer(ODE_INOUT ODE_MemoryBuffer *buffer, size_t length);

/// Destroys the memory buffer, freeing its allocated memory
ODE_Result ODE_API ode_destroyMemoryBuffer(ODE_INOUT ODE_MemoryBuffer *buffer);

#ifdef __cplusplus
}

#ifndef ODE_MINIMAL_API

// C++ utilities

#include <cstring>
#include <string>

/// Creates an ODE_StringRef from a C string - make sure that the input string outlives the reference!
inline ODE_StringRef ode_stringRef(const char *string) {
    ODE_StringRef ref = { };
    ref.data = reinterpret_cast<ODE_ConstCharPtr>(string);
    ref.length = int(strlen(string));
    return ref;
}

/// Creates an ODE_StringRef from a std::string - make sure that the input string outlives the reference!
inline ODE_StringRef ode_stringRef(const std::string &string) {
    ODE_StringRef ref = { };
    ref.data = reinterpret_cast<ODE_ConstCharPtr>(string.c_str());
    ref.length = int(string.size());
    return ref;
}

/// Creates an ODE_StringRef from an ODE_String
inline ODE_StringRef ode_stringRef(const ODE_String &string) {
    ODE_StringRef ref = { };
    ref.data = reinterpret_cast<ODE_ConstCharPtr>(string.data);
    ref.length = string.length;
    return ref;
}

/// Creates a std::string from an ODE_StringRef
inline std::string ode_stringDeref(const ODE_StringRef &ref) {
    return std::string(reinterpret_cast<const char *>(ref.data), ref.length);
}

/// Creates an ODE_String from a std::string
inline ODE_String ode_makeString(const std::string &str) {
    ODE_String string = { };
    if ((string.data = reinterpret_cast<char *>(malloc(str.size()+1)))) {
        memcpy(string.data, str.c_str(), str.size());
        string.data[str.size()] = '\0';
        string.length = int(str.size());
    }
    return string;
}

/// Creates an ODE_String from an ODE_StringRef
inline ODE_String ode_makeString(const ODE_StringRef &ref) {
    ODE_String string = { };
    if ((string.data = reinterpret_cast<char *>(malloc(ref.length+1)))) {
        memcpy(string.data, reinterpret_cast<const char *>(ref.data), ref.length);
        string.data[ref.length] = '\0';
        string.length = ref.length;
    }
    return string;
}

/// Creates an ODE_MemoryBuffer from an array
inline ODE_MemoryBuffer ode_makeMemoryBuffer(const void *data, size_t length) {
    ODE_MemoryBuffer buffer = { };
    if (length && (buffer.data = malloc(length))) {
        buffer.length = length;
        if (data)
            memcpy(buffer.data, data, length);
    }
    return buffer;
}

#endif
#endif

#endif // ODE_API_BASE_H
