
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_MEDIA_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-media.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_MEDIA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-media ${ODE_MEDIA_HEADERS} ${ODE_MEDIA_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ode FILES ${ODE_MEDIA_HEADERS} ${ODE_MEDIA_SOURCES})

# This module's include files
target_include_directories(ode-media INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# External dependencies
find_package(PNG REQUIRED)
if(NOT EMSCRIPTEN)
    find_package(JPEG REQUIRED)
    find_package(GIF REQUIRED)
    find_package(TIFF REQUIRED)
    find_package(WebP REQUIRED)
endif()

# Macro configuration
target_compile_definitions(ode-media PUBLIC ODE_MEDIA_PNG_SUPPORT)
if(NOT EMSCRIPTEN)
    target_compile_definitions(ode-media PUBLIC ODE_MEDIA_JPEG_SUPPORT)
    target_compile_definitions(ode-media PUBLIC ODE_MEDIA_GIF_SUPPORT)
    target_compile_definitions(ode-media PUBLIC ODE_MEDIA_TIFF_SUPPORT)
    target_compile_definitions(ode-media PUBLIC ODE_MEDIA_WEBP_SUPPORT)
endif()

# Module dependencies
target_link_libraries(ode-media PUBLIC ode-essentials)
target_link_libraries(ode-media PRIVATE PNG::PNG)
if(NOT EMSCRIPTEN)
    target_link_libraries(ode-media PRIVATE JPEG::JPEG TIFF::TIFF GIF::GIF WebP::webp WebP::webpdemux WebP::libwebpmux)
endif()
    # Note: Linking WebP::webpdemux & WebP::libwebpmux are only required by Skia - !!! TODO move to Rasterizer module
