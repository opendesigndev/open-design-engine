
cmake_minimum_required(VERSION 3.16)

# Gather sources
file(GLOB_RECURSE RENDERER_UNIT_TESTS_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h" "src/*.hpp")
file(GLOB_RECURSE RENDERER_UNIT_TESTS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

# EXECUTABLE
add_executable(renderer-unit-tests ${RENDERER_UNIT_TESTS_HEADERS} ${RENDERER_UNIT_TESTS_SOURCES})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${RENDERER_UNIT_TESTS_HEADERS} ${RENDERER_UNIT_TESTS_SOURCES})

# Module dependencies
target_link_libraries(renderer-unit-tests PRIVATE
    liboctopus
    ode-essentials
    ode-rasterizer
    ode-logic
    ode-media
    ode-graphics
    ode-renderer
    ode-diagnostics
)
