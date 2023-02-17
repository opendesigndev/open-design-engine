
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

    add_executable(ode-animation-prototype-module /dev/null) # TODO non-Unix compatibility

    target_link_libraries(ode-animation-prototype-module PRIVATE ode-animation-prototype)
    set_target_properties(ode-animation-prototype-module PROPERTIES OUTPUT_NAME ode)

    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s MODULARIZE=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s EXPORT_ES6=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s ALLOW_TABLE_GROWTH=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s ASSERTIONS=0")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s ENVIRONMENT=web")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s FILESYSTEM=0")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s USE_WEBGL2=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s USE_SDL=0")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s USE_ICU=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s USE_FREETYPE=1")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-s LLD_REPORT_UNDEFINED")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:--no-entry")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:--bind")
    target_link_options(ode-animation-prototype-module PRIVATE "SHELL:-Wl,--whole-archive")

    function(copy_to_npm_package from to)
        message("Copying ${from} to ${to}")
        add_custom_command(TARGET ode-animation-prototype-module POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${from}
            $<TARGET_FILE_DIR:ode-animation-prototype-module>/package/${to}
            DEPENDS ${from}
        )
    endfunction()

    copy_to_npm_package($<TARGET_FILE_DIR:ode-animation-prototype-module>/ode.js ode.js)
    copy_to_npm_package($<TARGET_FILE_DIR:ode-animation-prototype-module>/ode.wasm ode.wasm)
    copy_to_npm_package(${CMAKE_CURRENT_LIST_DIR}/npm/package.json package.json)
    copy_to_npm_package(${CMAKE_CURRENT_LIST_DIR}/npm/example.html ../index.html)

endif()
