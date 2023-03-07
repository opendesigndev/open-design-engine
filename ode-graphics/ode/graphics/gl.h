
#pragma once

#ifndef ODE_GRAPHICS_NO_CONTEXT

#include <cstdio>
#include <ode/utils.h>

#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__) || defined(__EMSCRIPTEN__)
    #include <GL/glew.h>
#else
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
#endif

#if defined(ODE_DEBUG)
    #include <cstdio>
    inline void _debug_breakable_print_if_glerror(const char *_file, int _line) {
        if (GLenum _opengl_err = glGetError())
            fprintf(stderr, "!!! OpenGL error %d (%s:%d)\n", (int) _opengl_err, _file, _line);
    }
    #define ODE_CHECK_GL_ERROR() { _debug_breakable_print_if_glerror(__FILE__, __LINE__); }
#else
    #define ODE_CHECK_GL_ERROR()
#endif

#if defined(_WIN32) || defined(ODE_GRAPHICS_OPENGL3_CORE)
    #define ODE_GL_ENABLE_VERTEX_ARRAYS
#elif defined(__EMSCRIPTEN__) ? defined(ODE_WEBGL1_COMPATIBILITY) : !defined(glBindSampler)
    #ifdef glBindSampler
        #undef glBindSampler
    #endif
    #define glBindSampler (static_cast<void (*)(int, decltype(nullptr))>(nullptr))
#endif

#else // ODE_GRAPHICS_NO_CONTEXT

#include <cstdint>

typedef uint32_t GLuint;

#endif // ODE_GRAPHICS_NO_CONTEXT
