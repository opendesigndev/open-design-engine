
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

# Module dependencies
target_link_libraries(ode-rasterizer PRIVATE skia)
target_link_libraries(ode-rasterizer PUBLIC liboctopus ode-essentials)
if(ODE_SKIA_GPU)
    target_link_libraries(ode-rasterizer PRIVATE ode-graphics)
endif()
