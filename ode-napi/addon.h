#pragma once
#define NODE_ADDON_API_DISABLE_DEPRECATED
// ^12.22.0 || ^14.17.0 || >=15.12.0, allows type_tag
#define NAPI_VERSION 8
// https://toyobayashi.github.io/emnapi-docs/guide/using-cpp.html
#define NAPI_DISABLE_CPP_EXCEPTIONS
#define NODE_ADDON_API_ENABLE_MAYBE
#include <napi.h>
#include <ode/api-base.h>
#include <optional>
#include "manual-api-base.h"

Napi::Object init_gen_api_base(Napi::Env env, Napi::Object exports);
Napi::Object init_gen_logic_api(Napi::Env env, Napi::Object exports);
Napi::Object init_gen_renderer_api(Napi::Env env, Napi::Object exports);

Napi::Object init_logic_api(Napi::Env env, Napi::Object exports);

// To be used in functions which return void/constructors
#define THROW_ON_ERROR(pred) if(check_result(env, pred)) return
// To be used in functions which return Napi::Value
#define THROW_ON_ERROR_V(pred) THROW_ON_ERROR(pred) Napi::Value()
bool check_result(Napi::Env env, ODE_Result result);

class Addon {
public:
    Addon(Napi::Object exports);

    Napi::ObjectReference exports;
    int new_counter;
    static Addon &from_env(const Napi::Env &);
};

bool copy_values(Napi::Value from, Napi::Value to);
template<typename T>
class Autobind {
public:
    static bool read_into(const Napi::Value &value, T &target);
    static bool read_into(const Napi::Maybe<Napi::Value> &value, T &target) {
        if (value.IsNothing()) return false;
        return Autobind<T>::read_into(value.Unwrap(), target);
    }
    static bool write_from(Napi::Value src, const T &target) {
        return copy_values(ode_napi_serialize(src.Env(), target), src);
    }
};

using ODE_Scalar_array_6 = ODE_Scalar[6];