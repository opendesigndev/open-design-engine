
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_GRAPHICS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-graphics.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_GRAPHICS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-graphics ${ODE_GRAPHICS_HEADERS} ${ODE_GRAPHICS_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ode FILES ${ODE_GRAPHICS_HEADERS} ${ODE_GRAPHICS_SOURCES})

# This module's include files
target_include_directories(ode-graphics INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Macro configuration
if(EMSCRIPTEN)
    target_compile_definitions(ode-graphics PUBLIC ODE_WEBGL_COMPATIBILITY)
    if(ODE_USE_WEBGL2)
        target_compile_definitions(ode-graphics PUBLIC ODE_USE_WEBGL2)
    else()
        target_compile_definitions(ode-graphics PUBLIC ODE_WEBGL1_COMPATIBILITY)
    endif()
else()
    target_compile_definitions(ode-graphics PUBLIC ODE_GRAPHICS_GLFW_CONTEXT)
endif()

# Module dependencies
if(NOT OVERRIDE_OPENGL_LIBRARY)
    target_link_libraries(ode-graphics PUBLIC OpenGL::GL)
endif()
if(NOT EMSCRIPTEN)
    target_link_libraries(ode-graphics PUBLIC glfw GLEW::GLEW)
endif()
target_link_libraries(ode-graphics PUBLIC ode-essentials)
