
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE ODE_LOGIC_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode-logic.h" "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE ODE_LOGIC_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp")

# LIBRARY
add_library(ode-logic ${ODE_LOGIC_HEADERS} ${ODE_LOGIC_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ode FILES ${ODE_LOGIC_HEADERS} ${ODE_LOGIC_SOURCES})

# This module's include files
target_include_directories(ode-logic INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Module dependencies
target_link_libraries(ode-logic PUBLIC liboctopus open-design-text-renderer ode-essentials ode-rasterizer)

# ODE version macros
if (DEFINED ODE_VERSION)
    target_compile_definitions(ode-logic PUBLIC ODE_VERSION=${ODE_VERSION})
endif()
if (DEFINED ODE_VERSION_MAJOR AND DEFINED ODE_VERSION_MINOR AND DEFINED ODE_VERSION_REVISION)
    target_compile_definitions(ode-logic PUBLIC ODE_VERSION_MAJOR=${ODE_VERSION_MAJOR})
    target_compile_definitions(ode-logic PUBLIC ODE_VERSION_MINOR=${ODE_VERSION_MINOR})
    target_compile_definitions(ode-logic PUBLIC ODE_VERSION_REVISION=${ODE_VERSION_REVISION})
endif()
if (DEFINED ODE_BUILD_COMMIT)
    target_compile_definitions(ode-logic PUBLIC ODE_BUILD_COMMIT=${ODE_BUILD_COMMIT})
endif()
if (DEFINED ODE_BUILD_DATE)
    target_compile_definitions(ode-logic PUBLIC ODE_BUILD_DATE=${ODE_BUILD_DATE})
endif()

# _CRT_SECURE_NO_WARNINGS
if (MSVC)
    target_compile_definitions(ode-logic PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

# EXECUTABLE
add_executable(ode-logic-cli "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(ode-logic-cli PRIVATE ode-logic)
