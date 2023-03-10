
cmake_minimum_required(VERSION 3.16)

set(ODE_ANIMATION_PROTOTYPE_WINDOW_SOURCES
    "window-main.cpp"
)

# LIBRARY
add_library(ode-animation-prototype INTERFACE)

# This module's include files
target_include_directories(ode-animation-prototype INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Module dependencies
target_link_libraries(ode-animation-prototype INTERFACE liboctopus ode-essentials ode-logic ode-renderer)

# EXECUTABLE
if(NOT EMSCRIPTEN)

    add_executable(ode-animation-prototype-window ${ODE_ANIMATION_PROTOTYPE_WINDOW_SOURCES})
    target_link_libraries(ode-animation-prototype-window PRIVATE glfw liboctopus ode-essentials ode-logic ode-graphics ode-animation-prototype)

else()
    message(WARN "Can't be used from the web. Use ode-napi instead")
endif()
