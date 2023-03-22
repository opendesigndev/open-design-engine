
# This script reads version from vcpkg.json (source of truth) and sets it to $ODE_VERSION etc.

file(STRINGS vcpkg.json ODE_VCPKG_JSON)

string(REGEX MATCH "\"version\"[ \t\n\r]*:[ \t\n\r]*\"[^\"]*\"" ODE_TMP_VERSION_PAIR ${ODE_VCPKG_JSON})
string(REGEX REPLACE "\"version\"[ \t\n\r]*:[ \t\n\r]*\"([^\"]*)\"" "\\1" ODE_VERSION ${ODE_TMP_VERSION_PAIR})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\1" ODE_VERSION_MAJOR ${ODE_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\2" ODE_VERSION_MINOR ${ODE_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\3" ODE_VERSION_REVISION ${ODE_VERSION})

string(REGEX MATCH "\"builtin-baseline\"[ \t\n\r]*:[ \t\n\r]*\"[^\"]*\"" ODE_TMP_BUILTIN_BASELINE_PAIR ${ODE_VCPKG_JSON})
string(REGEX REPLACE "\"builtin-baseline\"[ \t\n\r]*:[ \t\n\r]*\"([^\"]*)\"" "\\1" ODE_VCPKG_BUILTIN_BASELINE ${ODE_TMP_BUILTIN_BASELINE_PAIR})

if(NOT DEFINED ODE_BUILD_COMMIT_TAG)
    execute_process(COMMAND git describe --tags --always WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE ODE_BUILD_COMMIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(NOT DEFINED ODE_NPM_VERSION)
    if(DEFINED ENV{ODE_NPM_VERSION})
        set(ODE_NPM_VERSION $ENV{ODE_NPM_VERSION})
    else()
        set(ODE_NPM_VERSION ${ODE_BUILD_COMMIT_TAG})
    endif()
endif()
string(TIMESTAMP ODE_BUILD_DATE "%Y-%m-%d")

unset(ODE_TMP_VERSION_PAIR)
unset(ODE_TMP_BUILTIN_BASELINE_PAIR)
unset(ODE_VCPKG_JSON)
