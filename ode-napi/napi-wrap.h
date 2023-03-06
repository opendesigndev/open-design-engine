#pragma once
#include "addon.h"
#include <napi.h>
#include <optional>

template<typename T>
class Handle {
    static const char* name;
    static Napi::Value create(const Napi::CallbackInfo& info) {
        auto env = info.Env();
        if(info.Length() > 0) {
            Napi::Error::New(env, "No arguments expected").ThrowAsJavaScriptException();
            return Napi::Value();
        }
        Napi::Function obj = info.This().As<Napi::Function>();
        Addon &addon = Addon::from_env(env);
        if(!obj.InstanceOf(addon.exports.Get(Handle<T>::name).Unwrap().As<Napi::Function>()).Unwrap()) {
            Napi::Error::New(env, "Must be called with new").ThrowAsJavaScriptException();
            return Napi::Value();
        }
        auto ptr = new T();
        addon.new_counter++;
        // TODO: maybe we can actually remove this, but it IS good for debugging
        // we don't check it and instead rely on instanceof
        obj.Set(addon.engineSymbol.Value(), Napi::String::New(env, Handle<T>::name));
        obj.Set(addon.ptrSymbol.Value(), Napi::Number::New(env, (uintptr_t)ptr));
        return obj;
    }

    static Napi::Value del(const Napi::CallbackInfo& info) {
        auto env = info.Env();
        if(info.Length() > 0) {
            Napi::Error::New(env, "No arguments expected").ThrowAsJavaScriptException();
            return Napi::Value();
        }
        T* ptr = Handle<T>::Read_ptr(info.This());
        if(!ptr) return Napi::Value();

        Addon &addon = Addon::from_env(env);
        delete ptr;
        addon.new_counter--;
        return env.Undefined();
    }

public:
    static T* Read_ptr(const Napi::Value& value) {
        auto env = value.Env();
        Addon &addon = Addon::from_env(env);
        Napi::Function obj = value.As<Napi::Function>();
        if (env.IsExceptionPending()) return nullptr;
        if (!obj.InstanceOf(addon.exports.Get(Handle<T>::name).Unwrap().As<Napi::Function>()).Unwrap()) {
            Napi::Error::New(env, "Incorrect argument type (instanceof)").ThrowAsJavaScriptException();
            return nullptr;
        }
        auto maybe = obj.Get(addon.ptrSymbol.Value());
        if (env.IsExceptionPending()) return nullptr;
        if (maybe.IsNothing()) {
            Napi::Error::New(env, "Incorrect argument type (no ptr)").ThrowAsJavaScriptException();
            return nullptr;
        }
        auto ptr = (void*)(uintptr_t)(int64_t)maybe.Unwrap().As<Napi::Number>();
        if (ptr == nullptr) {
            Napi::Error::New(env, "Incorrect argument type (ptr is null)").ThrowAsJavaScriptException();
            return nullptr;
        }
        return static_cast<T*>(ptr);
    }

    static void Export(Napi::Object exports) {
        auto env = exports.Env();
        auto object = Napi::Function::New<Handle<T>::create>(env, Handle<T>::name);
        auto prototype = object.Get("prototype").Unwrap().template As<Napi::Object>();
        prototype.Set("delete", Napi::Function::New<Handle<T>::del>(env, "delete"));
        exports.Set(Handle<T>::name, object);
    }

    static std::optional<T> Read(const Napi::Value& value) {
        T* ptr = Handle<T>::Read_ptr(value);
        if (ptr == nullptr) return {};
        return *ptr;
    }
};
