
cmake_minimum_required(VERSION 3.16)

set(ODE_ANIMATION_PROTOTYPE_WINDOW_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/window-main.cpp"
)

if(NOT EMSCRIPTEN)
    add_executable(animation-prototype-window ${ODE_ANIMATION_PROTOTYPE_WINDOW_SOURCES})
    target_link_libraries(animation-prototype-window PRIVATE glfw liboctopus ode-essentials ode-logic ode-graphics ode-renderer)
endif()
