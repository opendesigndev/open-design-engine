
#pragma once

#define NODE_ADDON_API_DISABLE_DEPRECATED
// ^12.22.0 || ^14.17.0 || >=15.12.0, allows type_tag
#define NAPI_VERSION 8
// https://toyobayashi.github.io/emnapi-docs/guide/using-cpp.html
#define NAPI_DISABLE_CPP_EXCEPTIONS
#define NODE_ADDON_API_ENABLE_MAYBE

#include <napi.h>
