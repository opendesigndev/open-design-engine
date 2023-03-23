
cmake_minimum_required(VERSION 3.16)

file(GLOB ODE_NAPI_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.hpp")
file(GLOB ODE_NAPI_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

set(ODE_NAPI_GENERATED_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-api-base.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-logic-api.h
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-renderer-api.h
)

set(ODE_NAPI_GENERATED_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-api-base.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-logic-api.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-renderer-api.cpp
)

# LIBRARY / Emscripten module
if(EMSCRIPTEN)
    add_executable(ode-napi ${ODE_NAPI_HEADERS} ${ODE_NAPI_SOURCES} ${ODE_NAPI_GENERATED_HEADERS} ${ODE_NAPI_GENERATED_SOURCES})
else()
    add_library(ode-napi SHARED ${ODE_NAPI_HEADERS} ${ODE_NAPI_SOURCES} ${ODE_NAPI_GENERATED_HEADERS} ${ODE_NAPI_GENERATED_SOURCES})
endif()

# DEPENDENCIES
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# The intermediate target is required to avoid some dependencies being linked twice in wasm build
add_library(ode-napi-dependencies INTERFACE)
target_link_libraries(ode-napi-dependencies INTERFACE liboctopus ode-essentials ode-logic ode-renderer)
target_link_libraries(ode-napi PRIVATE ode-napi-dependencies)

# This module's include files
target_include_directories(ode-napi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(ode-napi PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
)

# Generate API bindings
set(ODE_NAPI_BINDING_GENERATOR_INPUT
    ${CMAKE_CURRENT_LIST_DIR}/../ode-essentials/ode/api-base.h
    ${CMAKE_CURRENT_LIST_DIR}/../ode-logic/ode/logic-api.h
    ${CMAKE_CURRENT_LIST_DIR}/../ode-renderer/ode/renderer-api.h
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/generated/index.d.ts
        ${CMAKE_CURRENT_BINARY_DIR}/generated/api-base.d.ts
        ${CMAKE_CURRENT_BINARY_DIR}/generated/logic-api.d.ts
        ${CMAKE_CURRENT_BINARY_DIR}/generated/renderer-api.d.ts
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-api-base.h
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-api-base.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-logic-api.h
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-logic-api.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-renderer-api.h
        ${CMAKE_CURRENT_BINARY_DIR}/generated/napi-renderer-api.cpp
    MAIN_DEPENDENCY
        ${CMAKE_CURRENT_LIST_DIR}/generate-api-bindings.py
    DEPENDS
        ${ODE_NAPI_BINDING_GENERATOR_INPUT}
        ${CMAKE_CURRENT_LIST_DIR}/../api-utils.py
        ${CMAKE_CURRENT_LIST_DIR}/generate-api-bindings.py
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/generate-api-bindings.py" "${CMAKE_CURRENT_BINARY_DIR}" ${ODE_NAPI_BINDING_GENERATOR_INPUT}
    VERBATIM
)

source_group("Generated source" FILES ${ODE_NAPI_GENERATED_HEADERS} ${ODE_NAPI_GENERATED_SOURCES})

# NPM PACKAGING
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
# - generated source files, which are only generated once
#
# Use correct function from the following list:
# copy_built_file_to_package, copy_generated_file_to_package, copy_source_file_to_package
# and first argument is always the target package or list of packages

function(copy_built_file_to_package packages file)
    copy_file_to_package("${packages}" $<TARGET_FILE_DIR:ode-napi>/${file} ${ARGV2})
endfunction()

function(copy_generated_file_to_package packages file)
    copy_file_to_package("${packages}" ${CMAKE_CURRENT_BINARY_DIR}/generated/${file} ${ARGV2})
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

    configure_file(wasm-package/index.js generated/index.js)
    copy_generated_file_to_package("target-specific" index.js)
    configure_file(wasm-package/package.json generated/target-specific/package.json)
    copy_generated_file_to_package("target-specific" target-specific/package.json)
    copy_built_file_to_package("target-specific" ode.js)
    copy_built_file_to_package("target-specific" ode.wasm)

    # configure_file does variable replacement
    configure_file(wrapper-package/package.json generated/package.json)
    configure_file(wrapper-package/node.js generated/node.js)
    configure_file(wrapper-package/wasm.js generated/wasm.js)
else()
    # Removes some warning
    set_target_properties(ode-napi PROPERTIES
        SUFFIX ".node"
        PREFIX ""
        POSITION_INDEPENDENT_CODE ON
    )

    # Module dependencies
    find_package(unofficial-node-addon-api CONFIG REQUIRED)
    target_link_libraries(ode-napi PUBLIC liboctopus ode-essentials ode-logic ode-renderer)
    target_link_libraries(ode-napi PRIVATE unofficial::node-addon-api::node-addon-api)

    if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
        # Generate node.lib
        execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
    endif()
    set(ODE_NATIVE_MODULE_PATH "./ode-napi.node")
    if(NOT CMAKE_JS_VERSION)
        # We are NOT in @opendesign/engine-source package, populate the package folder.
        if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(ODE_PLATFORM_OS "darwin")
            if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm64")
                set(ODE_PLATFORM_CPU "arm64")
            elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
                set(ODE_PLATFORM_CPU "x64")
            endif()
        endif()
    endif()

    configure_file(native-package/index.js generated/index.js)
    copy_generated_file_to_package("target-specific" index.js)
    if(DEFINED ODE_PLATFORM_OS AND DEFINED ODE_PLATFORM_CPU)
        configure_file(native-package/package.json generated/target-specific/package.json)
        copy_generated_file_to_package("target-specific" target-specific/package.json)
    endif()
    copy_built_file_to_package("target-specific" ode-napi.node)

    # configure_file does variable replacement
    configure_file(wrapper-package-test/package.json generated/package.json)
    configure_file(wrapper-package-test/node.js generated/node.js)
    configure_file(wrapper-package-test/wasm.js generated/wasm.js)
endif()

copy_generated_file_to_package("wrapper" wasm.js)
copy_generated_file_to_package("wrapper" package.json)
copy_generated_file_to_package("wrapper" node.js)

copy_generated_file_to_package("target-specific;wrapper" api-base.d.ts)
copy_generated_file_to_package("wrapper" index.d.ts node.d.ts)
copy_generated_file_to_package("wrapper" index.d.ts wasm.d.ts)
copy_generated_file_to_package("target-specific" index.d.ts)
copy_generated_file_to_package("target-specific;wrapper" logic-api.d.ts)
copy_generated_file_to_package("target-specific;wrapper" renderer-api.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/exports.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/common.d.ts)
copy_source_file_to_package("target-specific;wrapper" every-package/manual.d.ts)
