#pragma once
#include "addon.h"
#include <napi.h>
#include <optional>

template<typename T>
bool ode_napi_handle_read_into(const Napi::Value &value, const char *name, T &target) {
    auto env = value.Env();
    Napi::Object obj = value.As<Napi::Object>();
    if (env.IsExceptionPending()) return false;
    auto maybe = obj.Get(name);
    if (env.IsExceptionPending()) return false;
    if (maybe.IsNothing()) {
        Napi::Error::New(env, "Incorrect argument type (no Handle)").ThrowAsJavaScriptException();
        return false;
    }
    auto ptr = (void *) (uintptr_t) (int64_t) maybe.Unwrap().As<Napi::Number>();
    target.ptr = static_cast<decltype(target.ptr)>(ptr);
    return true;
}

template<typename T>
Napi::Value ode_napi_handle_serialize(Napi::Env env, const char *name, const T &source) {
    auto obj = Napi::Object::New(env);
    obj.Set(name, (uintptr_t) source.ptr);
    return obj;
}
