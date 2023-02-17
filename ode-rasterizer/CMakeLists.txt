
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_RASTERIZER_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-rasterizer.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_RASTERIZER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-rasterizer ${ODE_RASTERIZER_HEADERS} ${ODE_RASTERIZER_SOURCES})

# This module's include files
target_include_directories(ode-rasterizer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Macro configuration
if(ODE_SKIA_GPU)
    target_compile_definitions(ode-rasterizer PUBLIC ODE_RASTERIZER_TEXTURE_SUPPORT)
    target_compile_definitions(ode-rasterizer PRIVATE SK_GL)
endif()

# Issue with new version of vcpkg's Skia - different include directory
if(NOT EMSCRIPTEN)
    find_path(ODE_SKIA_INCLUDE_DIR "skia/core/SkPath.h")
    target_include_directories(ode-rasterizer PRIVATE ${ODE_SKIA_INCLUDE_DIR})
endif()

# Module dependencies
target_link_libraries(ode-rasterizer PRIVATE skia)
target_link_libraries(ode-rasterizer PUBLIC liboctopus ode-essentials)
if(ODE_SKIA_GPU)
    target_link_libraries(ode-rasterizer PUBLIC ode-graphics)
    if(NOT OVERRIDE_OPENGL_LIBRARY)
        target_link_libraries(ode-rasterizer PRIVATE OpenGL::GL)
    endif()
    if(NOT EMSCRIPTEN)
        target_link_libraries(ode-rasterizer PRIVATE GLEW::GLEW)
    endif()
endif()
