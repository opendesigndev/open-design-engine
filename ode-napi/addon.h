#pragma once
#define NODE_ADDON_API_DISABLE_DEPRECATED
// ^12.22.0 || ^14.17.0 || >=15.12.0, allows type_tag
#define NAPI_VERSION 8
// https://toyobayashi.github.io/emnapi-docs/guide/using-cpp.html
#define NAPI_DISABLE_CPP_EXCEPTIONS
#define NODE_ADDON_API_ENABLE_MAYBE
#include <napi.h>
#include <ode/api-base.h>
#include <cstdlib>
#include <optional>
#include "manual-api-base.h"
#include "manual-logic-api.h"

Napi::Object init_gen_api_base(Napi::Env env, Napi::Object exports);
Napi::Object init_gen_logic_api(Napi::Env env, Napi::Object exports);
Napi::Object init_gen_renderer_api(Napi::Env env, Napi::Object exports);

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

using ODE_Scalar_array_6 = ODE_Scalar[6];

Napi::Object init_api_base(Napi::Env env, Napi::Object exports);
Napi::Value ode_napi_serialize(Napi::Env env, const int &value);
Napi::Value ode_napi_serialize(Napi::Env env, const size_t &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_ConstDataPtr &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_VarDataPtr &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Scalar &value);
bool ode_napi_read_into(const Napi::Value &value, int &parsed);
bool ode_napi_read_into(const Napi::Value &value, size_t &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_VarDataPtr &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_Scalar &parsed);

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_MemoryBuffer &value);
bool ode_napi_read_into(const Napi::Value &value, ODE_MemoryBuffer &target);

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_String &value);


template<typename T>
inline bool ode_napi_serialize(Napi::Env env, const T *&parsed) {
    return ode_napi_serialize(env, reinterpret_cast<ODE_ConstDataPtr>(parsed));
}

template<typename T>
inline bool ode_napi_read_into(const Napi::Value &value, T *&parsed) {
    uintptr_t target;
    if (ode_napi_read_into(value, target)) {
        parsed = reinterpret_cast<T *>(target);
        return true;
    }
    return false;
}

template<typename T>
inline bool ode_napi_read_into(const Napi::Maybe<Napi::Value> &value, T &target) {
    if (value.IsNothing()) return false;
    return ode_napi_read_into(value.Unwrap(), target);
}

bool copy_values(Napi::Value from, Napi::Value to);
template<typename T>
inline bool ode_napi_write_from(Napi::Value target, const T &src) {
    return copy_values(ode_napi_serialize(target.Env(), src), target);
}
