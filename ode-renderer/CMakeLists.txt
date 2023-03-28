
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_RENDERER_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-renderer.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_RENDERER_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-renderer ${ODE_RENDERER_HEADERS} ${ODE_RENDERER_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ode FILES ${ODE_RENDERER_HEADERS} ${ODE_RENDERER_SOURCES})

# This module's include files
target_include_directories(ode-renderer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Module configuration
target_compile_definitions(ode-renderer PRIVATE ODE_REALTIME_TEXT_RENDERER)

# Module dependencies
target_link_libraries(ode-renderer PUBLIC liboctopus open-design-text-renderer ode-essentials ode-rasterizer ode-logic ode-graphics)
if(NOT EMSCRIPTEN)
    target_link_libraries(ode-renderer PUBLIC ode-media)
endif()

# EXECUTABLE
if(NOT EMSCRIPTEN)
    add_executable(ode-renderer-cli "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    target_link_libraries(ode-renderer-cli PRIVATE ode-renderer)
endif()
