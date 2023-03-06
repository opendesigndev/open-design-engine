#include "addon.h"
#include <ode/api-base.h>
#include "napi-wrap.h"
Napi::Value bind_ode_makeString(const Napi::CallbackInfo& info);

Napi::Object init_api_base(Napi::Env env, Napi::Object exports) {
    exports.Set("makeString", Napi::Function::New<bind_ode_makeString>(env, "makeString"));
    return exports;
}

template<>
bool Autobind<int>::read_into(const Napi::Value& value, int& parsed){
    parsed = (int32_t)value.As<Napi::Number>();
    return true;
}

template<>
bool Autobind<unsigned long>::read_into(const Napi::Value& value, unsigned long& parsed){
    parsed = (int64_t)value.As<Napi::Number>();
    return true;
}

template<>
bool Autobind<ODE_Vector2>::read_into(const Napi::Value& value, ODE_Vector2& parsed){
    Napi::Array arr = value.As<Napi::Array>();
    parsed.x = (int32_t)arr.Get(uint32_t(0)).Unwrap().As<Napi::Number>();
    parsed.y = (int32_t)arr.Get(uint32_t(1)).Unwrap().As<Napi::Number>();
    return true;
}

template<>
bool Autobind<ODE_Rectangle>::read_into(const Napi::Value& value, ODE_Rectangle& parsed){
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_Rectangle>::read_into").ThrowAsJavaScriptException();
    return false;
}

template<>
bool Autobind<ODE_ConstDataPtr>::read_into(const Napi::Value& value, ODE_ConstDataPtr& parsed){
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_ConstDataPtr>::read_into").ThrowAsJavaScriptException();
    return false;
}


template<>
bool Autobind<ODE_VarDataPtr>::read_into(const Napi::Value& value, ODE_VarDataPtr& parsed){
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_VarDataPtr>::read_into").ThrowAsJavaScriptException();
    return false;
}

template<>
bool Autobind<ODE_Scalar>::read_into(const Napi::Value& value, ODE_Scalar& parsed){
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_Scalar>::read_into").ThrowAsJavaScriptException();
    return false;
}

Napi::Value bind_ode_makeString(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    auto string = ode_makeString(info[0].As<Napi::String>());
    auto obj = Napi::Object::New(env);
    obj.Set("data", Napi::Number::New(env, (uintptr_t)string.data));
    obj.Set("length", Napi::Number::New(env, string.length));
    return obj;
}

