
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

# Real-time text renderer dependencies
if(ODE_REALTIME_TEXT)
    set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE INTERNAL "" FORCE)
    set(MSDF_ATLAS_USE_VCPKG OFF CACHE INTERNAL "" FORCE)
    set(MSDF_ATLAS_USE_SKIA ON CACHE INTERNAL "" FORCE)
    set(MSDF_ATLAS_NO_ARTERY_FONT ON CACHE INTERNAL "" FORCE)
    set(MSDF_ATLAS_INSTALL OFF CACHE INTERNAL "" FORCE)
    set(MSDF_ATLAS_DYNAMIC_RUNTIME OFF CACHE INTERNAL "" FORCE)
    if(NOT EMSCRIPTEN)
        set(MSDF_ATLAS_MSDFGEN_EXTERNAL ON CACHE INTERNAL "" FORCE)
    else()
        set(MSDF_ATLAS_MSDFGEN_EXTERNAL OFF CACHE INTERNAL "" FORCE)
        set(MSDFGEN_DISABLE_SVG ON)
        set(MSDFGEN_DISABLE_VARIABLE_FONTS ON)
    endif()
    add_subdirectory(../third-party/msdf-atlas-gen ${CMAKE_CURRENT_BINARY_DIR}/msdf-atlas-gen)
    target_link_libraries(ode-renderer PRIVATE msdf-atlas-gen)
    target_compile_definitions(ode-renderer PUBLIC ODE_REALTIME_TEXT_RENDERER)
    target_link_libraries(ode-renderer PRIVATE freetype harfbuzz::harfbuzz)
endif()

# Module dependencies
target_link_libraries(ode-renderer PUBLIC liboctopus open-design-text-renderer ode-essentials ode-rasterizer ode-logic ode-graphics ode-media)

# EXECUTABLE
if(NOT EMSCRIPTEN)
    add_executable(ode-renderer-cli "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    target_link_libraries(ode-renderer-cli PRIVATE ode-renderer)
endif()
