
cmake_minimum_required(VERSION 3.16)

set(ODE_NAPI_SOURCES
    "addon.cpp"
    "gen-api-base.cpp"
    "gen-logic-api.cpp"
    "gen-renderer-api.cpp"
    "api-base.cpp"
    "logic-api.cpp"
)

# LIBRARY
add_library(ode-napi SHARED ${ODE_NAPI_SOURCES})
set_target_properties(ode-napi PROPERTIES
    SUFFIX ".node"
    PREFIX ""
    POSITION_INDEPENDENT_CODE ON)

# This module's include files
target_include_directories(ode-napi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Removes some warning
set_target_properties(ode-napi PROPERTIES
    SUFFIX ".node"
    PREFIX ""
    POSITION_INDEPENDENT_CODE ON)

# Module dependencies
find_package(unofficial-node-addon-api CONFIG REQUIRED)
target_link_libraries(ode-napi PUBLIC liboctopus ode-essentials ode-logic ode-renderer)
target_link_libraries(ode-napi PRIVATE unofficial::node-addon-api::node-addon-api)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
    # Generate node.lib
    execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()
