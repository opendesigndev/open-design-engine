
cmake_minimum_required(VERSION 3.16)

project(design-editor LANGUAGES C CXX)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party)
# Definitions of third-party libraries directories
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
set(IMGUI_FILEDIALOG_DIR ${THIRD_PARTY_DIR}/ImGuiFileDialog)

set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

set(IMGUI_FILEDIALOG_HEADERS
    ${IMGUI_FILEDIALOG_DIR}/ImGuiFileDialog.h
)
set(IMGUI_FILEDIALOG_SOURCES
    ${IMGUI_FILEDIALOG_DIR}/ImGuiFileDialog.cpp
)

# Gather sources
file(GLOB_RECURSE DESING_EDITOR_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.h" "ode/*.hpp")
file(GLOB_RECURSE DESING_EDITOR_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ode/*.cpp" "main.cpp")

file(GLOB_RECURSE IMGUI_COMPATIBILITY_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "imgui/*.h")
file(GLOB_RECURSE IMGUI_COMPATIBILITY_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "imgui/*.cpp")

# EXECUTABLE
add_executable(${PROJECT_NAME}
    ${DESING_EDITOR_HEADERS} ${DESING_EDITOR_SOURCES}
    ${IMGUI_HEADERS} ${IMGUI_SOURCES}
    ${IMGUI_COMPATIBILITY_HEADERS} ${IMGUI_COMPATIBILITY_SOURCES}
    ${IMGUI_FILEDIALOG_HEADERS} ${IMGUI_FILEDIALOG_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends ${IMGUI_FILEDIALOG_DIR} ${IMNODES_DIR})

# IDE tree hierarchy
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DESING_EDITOR_HEADERS} ${DESING_EDITOR_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMGUI_COMPATIBILITY_HEADERS} ${IMGUI_COMPATIBILITY_SOURCES})
source_group(TREE ${IMGUI_DIR}/.. FILES ${IMGUI_HEADERS} ${IMGUI_SOURCES})
source_group(TREE ${IMGUI_FILEDIALOG_DIR}/.. FILES ${IMGUI_FILEDIALOG_HEADERS} ${IMGUI_FILEDIALOG_SOURCES})

# Required Open Desing Engine internal libraries
set(INTERNAL_LIBRARY_DEPENDENCIES
    liboctopus
    ode-essentials
    ode-rasterizer
    ode-logic
    ode-media
    ode-graphics
    ode-renderer
    ode-diagnostics
)

# Module dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC ${INTERNAL_LIBRARY_DEPENDENCIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS NO
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 # Warning level 4
        # /WX # Treat warnings as errors
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        # -Werror # Treat warnings as errors
        -Wshadow
        -pedantic
    >
)
