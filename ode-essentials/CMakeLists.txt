
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_ESSENTIALS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-essentials.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_ESSENTIALS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-essentials ${ODE_ESSENTIALS_HEADERS} ${ODE_ESSENTIALS_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ode FILES ${ODE_ESSENTIALS_HEADERS} ${ODE_ESSENTIALS_SOURCES})

# This module's include files
target_include_directories(ode-essentials INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
