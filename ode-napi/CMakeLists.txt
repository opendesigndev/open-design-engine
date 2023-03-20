
cmake_minimum_required(VERSION 3.16)

set(ODE_NAPI_SOURCES
    "addon.cpp"
    ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-api-base.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-logic-api.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-renderer-api.cpp
    "manual-api-base.cpp"
    "manual-logic-api.cpp"
    "addon.h"
    "manual-logic-api.h"
    "napi-wrap.h"
)

# LIBRARY / EXECUTABLE
if(EMSCRIPTEN)
    add_executable(ode-napi ${ODE_NAPI_SOURCES})
else()
    add_library(ode-napi SHARED ${ODE_NAPI_SOURCES})
endif()

# DEPENDENCIES
# The intermediate target is required to avoid some dependencies being linked
# twice in wasm build.
add_library(ode-napi-lib INTERFACE)
target_link_libraries(ode-napi-lib INTERFACE liboctopus ode-essentials ode-logic ode-renderer)
target_link_libraries(ode-napi PRIVATE ode-napi-lib)

# This module's include files
target_include_directories(ode-napi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(ode-napi PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_CURRENT_BINARY_DIR}/codegen/
)

SET(ODE_NAPI_CODEGEN_INPUT
    ../ode-essentials/ode/api-base.h
    ../ode-logic/ode/logic-api.h
    ../ode-renderer/ode/renderer-api.h
)

add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/api-base.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/index.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/logic-api.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/renderer-api.d.ts
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-api-base.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-api-base.h
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-logic-api.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-logic-api.h
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-renderer-api.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen-renderer-api.h
            ${CMAKE_CURRENT_BINARY_DIR}/codegen/gen.h
        MAIN_DEPENDENCY
            generate-api-bindings.py
        DEPENDS
            ${ODE_NAPI_CODEGEN_INPUT}
            generate-api-bindings.py
        COMMAND python3 "${CMAKE_CURRENT_LIST_DIR}/generate-api-bindings.py" "${CMAKE_CURRENT_BINARY_DIR}" ${ODE_NAPI_CODEGEN_INPUT}
        VERBATIM
)

function(copy_file_to_package packages from)
    set(target ${ARGV2})
    if(NOT target)
        get_filename_component(target "${from}" NAME)
    endif()
    foreach(package IN LISTS packages)
        set(avail_packages "target-specific;wrapper")
        if(NOT ${package} IN_LIST avail_packages)
            message(FATAL_ERROR "Invalid package ${package}")
        endif()

        add_custom_command(TARGET ode-napi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${from}
            $<TARGET_FILE_DIR:ode-napi>/${package}/${target}
            DEPENDS ${from}
        )
    endforeach()
endfunction()

# There are two packages generated:
# - wrapper, which just reexports native and wasm so that one package could
#   be used in both web and nodejs
#   - the real one gets generated from wasm build, one for local testing is
#     generated from native build
# - native/wasm package - target-specific one which contains the real file
# - there is also source only package which basically contains all sources from
#   this repo but that one is not generate using cmake.
#
# Then there are three sources for files which can be put into those packages:
# - source files
# - binary files, which are generated per config in case of cmake multi config setup
# - codegened files, which are only generated once
#
# Use correct function from the following list:
# copy_built_file_to_package, copy_codegen_file_to_package, copy_source_file_to_package
# and first argument is always the target package or list of packages

function(copy_built_file_to_package packages file)
    copy_file_to_package("${packages}" $<TARGET_FILE_DIR:ode-napi>/${file} ${ARGV2})
endfunction()

function(copy_codegen_file_to_package packages file)
    copy_file_to_package("${packages}" ${CMAKE_CURRENT_BINARY_DIR}/codegen/${file} ${ARGV2})
endfunction()

function(copy_source_file_to_package packages file)
    copy_file_to_package("${packages}" ${CMAKE_CURRENT_LIST_DIR}/${file} ${ARGV2})
endfunction()

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
    target_link_options(ode-napi PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=\"['_malloc','_free']\"")
    target_link_options(ode-napi PRIVATE "SHELL:--no-entry")
    target_link_options(ode-napi PRIVATE "SHELL:--bind")
    target_link_options(ode-napi PRIVATE "SHELL:-Wl,--whole-archive")
    if(ODE_USE_WEBGL2)
        target_link_options(ode-napi PRIVATE "SHELL:-s USE_WEBGL2=1")
    endif()

    configure_file(wasm-package/index.js codegen/index.js)
    copy_codegen_file_to_package("target-specific" index.js)
    configure_file(wasm-package/package.json codegen/target-specific/package.json)
    copy_codegen_file_to_package("target-specific" target-specific/package.json)
    copy_built_file_to_package("target-specific" ode.js)
    copy_built_file_to_package("target-specific" ode.wasm)

    # configure_file does variable replacement
    configure_file(wrapper-package/package.json codegen/package.json)
    configure_file(wrapper-package/node.js codegen/node.js)
    configure_file(wrapper-package/wasm.js codegen/wasm.js)
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
    if(CMAKE_JS_VERSION)
        # We ARE in @opendesign/engine-source package
        set(ODE_NATIVE_MODULE_PATH "../ode-napi.node")
    else()
        # We are NOT in @opendesign/engine-source package, populate the package folder.
        set(ODE_NATIVE_MODULE_PATH "./ode-napi.node")
        if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(ODE_PLATFORM_OS "darwin")
            if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
                set(ODE_PLATFORM_CPU "arm64")
            elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
                set(ODE_PLATFORM_CPU "x64")
            endif()
        endif()
    endif()
    
    if(DEFINED ODE_PLATFORM_OS AND DEFINED ODE_PLATFORM_CPU)
        configure_file(native-package/index.js codegen/index.js)
        copy_codegen_file_to_package("target-specific" index.js)
        configure_file(native-package/package.json codegen/target-specific/package.json)
        copy_codegen_file_to_package("target-specific" target-specific/package.json)
    endif()
    copy_built_file_to_package("target-specific" ode-napi.node)
    
    # configure_file does variable replacement
    configure_file(wrapper-package-test/package.json codegen/package.json)
    configure_file(wrapper-package-test/node.js codegen/node.js)
    configure_file(wrapper-package-test/wasm.js codegen/wasm.js)
endif()


copy_codegen_file_to_package("wrapper" wasm.js)
copy_codegen_file_to_package("wrapper" package.json)
copy_codegen_file_to_package("wrapper" node.js)

copy_codegen_file_to_package("target-specific;wrapper" api-base.d.ts)
copy_codegen_file_to_package("wrapper" index.d.ts node.d.ts)
copy_codegen_file_to_package("wrapper" index.d.ts wasm.d.ts)
copy_codegen_file_to_package("target-specific" index.d.ts)
copy_codegen_file_to_package("target-specific;wrapper" logic-api.d.ts)
copy_codegen_file_to_package("target-specific;wrapper" renderer-api.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/exports.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/common.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/manual.d.ts)
