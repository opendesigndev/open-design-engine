
cmake_minimum_required(VERSION 3.16)

set(ODE_NAPI_SOURCES
    "addon.cpp"
    ${CMAKE_CURRENT_BINARY_DIR}/src/gen-api-base.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/gen-logic-api.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/gen-renderer-api.cpp
    "manual-api-base.cpp"
    "manual-logic-api.cpp"
)

# LIBRARY / EXECUTABLE
if(EMSCRIPTEN)
    add_executable(ode-napi ${ODE_NAPI_SOURCES})
else()
    add_library(ode-napi SHARED ${ODE_NAPI_SOURCES})
endif()

# dependencies
add_library(ode-napi-lib INTERFACE )
target_link_libraries(ode-napi-lib INTERFACE liboctopus ode-essentials ode-logic ode-renderer)
target_link_libraries(ode-napi PRIVATE ode-napi-lib)

# This module's include files
target_include_directories(ode-napi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(ode-napi PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

SET(ODE_NAPI_CODEGEN_INPUT
    ../ode-essentials/ode/api-base.h
    ../ode-logic/ode/logic-api.h
    ../ode-renderer/ode/renderer-api.h
)

add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/package/api-base.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/package/common.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/package/exports.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/package/index.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/package/logic-api.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/src/gen-api-base.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/src/gen-logic-api.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/src/gen-renderer-api.cpp
        MAIN_DEPENDENCY
            generate-api-bindings.py
        DEPENDS
            ${ODE_NAPI_CODEGEN_INPUT}
            generate-api-bindings.py
        COMMAND python3 "${CMAKE_CURRENT_LIST_DIR}/generate-api-bindings.py" "${CMAKE_CURRENT_BINARY_DIR}" ${ODE_NAPI_CODEGEN_INPUT}
        VERBATIM
)

if(EMSCRIPTEN)
    # Enable emnapi
    include(FetchContent)
    # You can get the values by running
    # > npm view emnapi dist.tarball
    # > npm view emnapi dist.integrity
    # Note that you'll have to convert the hash from base64 to hex
    FetchContent_Declare(emnapi
        URL https://registry.npmjs.org/emnapi/-/emnapi-0.32.2.tgz
        URL_HASH SHA512=87932c3bc03d34360883c302ab5b88bc4e2f400be12e074054059e6bc9ae6378ee448f469ae907460f8f23445eb92b80dde66919f2bd0535bee2f2a140fa616a
    )
    FetchContent_MakeAvailable(emnapi)
    target_link_libraries(ode-napi PRIVATE emnapi)


    set_target_properties(ode-napi PROPERTIES OUTPUT_NAME ode)

    # Emscripten options
    target_link_options(ode-napi PRIVATE "SHELL:-s MODULARIZE=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s EXPORT_ES6=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s ALLOW_TABLE_GROWTH=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s ASSERTIONS=0")
    target_link_options(ode-napi PRIVATE "SHELL:-s ENVIRONMENT=web")
    target_link_options(ode-napi PRIVATE "SHELL:-s FILESYSTEM=0")
    target_link_options(ode-napi PRIVATE "SHELL:-s USE_SDL=0")
    target_link_options(ode-napi PRIVATE "SHELL:-s USE_ICU=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s USE_FREETYPE=1")
    target_link_options(ode-napi PRIVATE "SHELL:-s LLD_REPORT_UNDEFINED")
    target_link_options(ode-napi PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS=malloc,free")
    target_link_options(ode-napi PRIVATE "SHELL:--no-entry")
    target_link_options(ode-napi PRIVATE "SHELL:--bind")
    target_link_options(ode-napi PRIVATE "SHELL:-Wl,--whole-archive")
    if(ODE_USE_WEBGL2)
        target_link_options(ode-napi PRIVATE "SHELL:-s USE_WEBGL2=1")
    endif()

    function(copy_to_npm_package file)
        add_custom_command(TARGET ode-napi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:ode-napi>/${file}
            $<TARGET_FILE_DIR:ode-napi>/package/${file}
            DEPENDS $<TARGET_FILE_DIR:ode-napi>/${file}
        )
    endfunction()

    configure_file(wasm-package/package.json package.json)
    configure_file(wasm-package/index.js index.js)
    copy_to_npm_package(ode.js)
    copy_to_npm_package(ode.wasm)
    copy_to_npm_package(package.json)
    copy_to_npm_package(index.js)
    configure_file(typescript-bindings/internal.d.ts ${CMAKE_CURRENT_BINARY_DIR}/package/internal.d.ts COPYONLY)
else()
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
endif()
