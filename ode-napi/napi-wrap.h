#pragma once
#include "addon.h"
#include <napi.h>
#include <optional>

template<typename T>
class Handle {
    static const char *name;
    static Napi::Value create(const Napi::CallbackInfo &info) {
        auto env = info.Env();
        if (info.Length() > 0) {
            Napi::Error::New(env, "No arguments expected").ThrowAsJavaScriptException();
            return Napi::Value();
        }
        Napi::Function obj = info.This().As<Napi::Function>();
        Addon &addon = Addon::from_env(env);
        if (!obj.InstanceOf(addon.exports.Get(Handle<T>::name).Unwrap().As<Napi::Function>()).Unwrap()) {
            Napi::Error::New(env, "Must be called with new").ThrowAsJavaScriptException();
            return Napi::Value();
        }

        obj.Set(Handle<T>::name, Napi::Number::New(env, 0));
        return obj;
    }

public:
    static std::optional<T> Read(const Napi::Value &value) {
        auto env = value.Env();
        Addon &addon = Addon::from_env(env);
        if (value.IsNull()) return T {};
        Napi::Function obj = value.As<Napi::Function>();
        if (env.IsExceptionPending()) return {};
        auto maybe = obj.Get(Handle<T>::name);
        if (env.IsExceptionPending()) return {};
        if (maybe.IsNothing()) {
            Napi::Error::New(env, "Incorrect argument type (no Handle)").ThrowAsJavaScriptException();
            return {};
        }
        auto ptr = (void *) (uintptr_t) (int64_t) maybe.Unwrap().As<Napi::Number>();
        T result;
        result.ptr = static_cast<decltype(result.ptr)>(ptr);
        return result;
    }

    static Napi::Value serialize(Napi::Env env, T handle) {
        auto obj = Napi::Object::New(env);
        obj.Set(Handle<T>::name, (uintptr_t) handle.ptr);
        return obj;
    }

    static void Export(Napi::Object exports) {
        auto env = exports.Env();
        auto object = Napi::Function::New<Handle<T>::create>(env, Handle<T>::name);
        exports.Set(Handle<T>::name, object);
    }
};
