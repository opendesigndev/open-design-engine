
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_DIAGNOSTICS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "diag/*.h" "diag/*.hpp")
file(GLOB_RECURSE ODE_DIAGNOSTICS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "diag/*.cpp")

# LIBRARY
add_library(ode-diagnostics "ode-diagnostics.h" ${ODE_DIAGNOSTICS_HEADERS} ${ODE_DIAGNOSTICS_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/diag FILES ${ODE_DIAGNOSTICS_HEADERS} ${ODE_DIAGNOSTICS_SOURCES})

# This module's include files
target_include_directories(ode-diagnostics INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Module dependencies
target_link_libraries(ode-diagnostics PUBLIC liboctopus ode-essentials ode-rasterizer ode-logic ode-media ode-graphics ode-renderer)
