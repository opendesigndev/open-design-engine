
#pragma once

// Determine release (ODE_RELEASE) or debug (ODE_DEBUG) mode
#if defined(NDEBUG) || (defined(_MSC_VER) && !defined(_DEBUG))
    #define ODE_RELEASE 1
#else
    #define ODE_DEBUG 1
#endif

// Debug runtime assertion
#ifdef ODE_DEBUG
    #ifdef _MSC_VER
        #include <crtdbg.h>
        #define ODE_ASSERT _ASSERT
    #else
        #include <cassert>
        #define ODE_ASSERT assert
    #endif
#else
    #define ODE_ASSERT(...)
#endif

/// Turns a sequence of statements into what is syntactically considered a single statement
#define ODE_STATEMENT(...) do { __VA_ARGS__; } while (0)

/// Turns a value into a string literal
#define ODE_STRINGIZE(...) ODE_STRINGIZE_INTERNAL(__VA_ARGS__)
#define ODE_STRINGIZE_INTERNAL(...) #__VA_ARGS__

namespace ode {

/// A byte is an unsigned 8-bit integer
typedef unsigned char byte;

}
