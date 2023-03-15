#include "addon.h"
#include <ode/api-base.h>
#include "napi-wrap.h"
#include "gen.h"

Napi::Value node_napi_makeString(const Napi::CallbackInfo &info);
Napi::Value node_napi_readString(const Napi::CallbackInfo &info);

Napi::Object init_api_base(Napi::Env env, Napi::Object exports) {
    exports.Set("makeString", Napi::Function::New<node_napi_makeString>(env, "makeString"));
    exports.Set("readString", Napi::Function::New<node_napi_readString>(env, "readString"));
    return exports;
}

bool ode_napi_read_into(const Napi::Value &value, int &parsed) {
    parsed = (int32_t) value.As<Napi::Number>();
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const int &value) {
    return Napi::Number::New(env, value);
}


bool ode_napi_read_into(const Napi::Value &value, unsigned long &parsed) {
    parsed = (int64_t) value.As<Napi::Number>();
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const unsigned long &value) {
    auto sgn = static_cast<int64_t>(value);
    if (static_cast<unsigned long>(sgn) != value || sgn < 0) {
        Napi::Error::New(env, "Too big to convert to js number").ThrowAsJavaScriptException();
        return Napi::Value();
    }
    return Napi::Number::New(env, (uint32_t) value);
}


bool ode_napi_read_into(const Napi::Value &value, ODE_Vector2 &parsed) {
    Napi::Array arr = value.As<Napi::Array>();
    parsed.x = (ODE_Scalar) arr.Get(uint32_t(0)).Unwrap().As<Napi::Number>();
    parsed.y = (ODE_Scalar) arr.Get(uint32_t(1)).Unwrap().As<Napi::Number>();
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Vector2 &parsed) {
    Napi::Error::New(env, "Not implemented: ode_napi_serialize(ODE_Vector2)").ThrowAsJavaScriptException();
    return Napi::Value();
}


bool ode_napi_read_into(const Napi::Value &value, ODE_Rectangle &parsed) {
    Napi::Error::New(value.Env(), "Not implemented: ode_napi_read_into(ODE_Rectangle)").ThrowAsJavaScriptException();
    return false;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Rectangle &parsed) {
    Napi::Error::New(env, "Not implemented: ode_napi_serialize(ODE_Rectangle)").ThrowAsJavaScriptException();
    return Napi::Value();
}


bool ode_napi_read_into(const Napi::Value &value, ODE_ConstDataPtr &parsed) {
    Napi::Error::New(value.Env(), "Not implemented: ode_napi_read_into(ODE_ConstDataPtr)").ThrowAsJavaScriptException();
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_ConstDataPtr &parsed) {
    Napi::Error::New(env, "Not implemented: ode_napi_serialize(ODE_ConstDataPtr)").ThrowAsJavaScriptException();
    return Napi::Value();
}


bool ode_napi_read_into(const Napi::Value &value, ODE_VarDataPtr &parsed) {
    Napi::Error::New(value.Env(), "Not implemented: ode_napi_read_into(ODE_VarDataPtr)").ThrowAsJavaScriptException();
    return false;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_VarDataPtr &parsed) {
    Napi::Error::New(env, "Not implemented: ode_napi_serialize(ODE_VarDataPtr)").ThrowAsJavaScriptException();
    return Napi::Value();
}


bool ode_napi_read_into(const Napi::Value &value, ODE_Scalar &parsed) {
    parsed = (ODE_Scalar) value.As<Napi::Number>();
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Scalar &parsed) {
    Napi::Error::New(env, "Not implemented: ode_napi_serialize(ODE_Scalar)").ThrowAsJavaScriptException();
    return Napi::Value();
}


Napi::Value node_napi_makeString(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto string = ode_makeString(info[0].As<Napi::String>());
    auto obj = Napi::Object::New(env);
    obj.Set("data", Napi::Number::New(env, (uintptr_t) string.data));
    obj.Set("length", Napi::Number::New(env, string.length));
    return obj;
}

Napi::Value node_napi_readString(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    ODE_StringRef ref;
    if (!ode_napi_read_into(info[0], ref)) {
        auto ex = env.GetAndClearPendingException();
        Napi::Error::New(env, "Failed to parse argument string ("+ ex.Message() +")").ThrowAsJavaScriptException();
        return Napi::Value();
    }
    return Napi::String::New(env, ref.data, ref.length);
}

bool ode_napi_read_into(const Napi::Value &value, double(&parsed)[6]) {
    Napi::Array obj = value.As<Napi::Array>();
    for (auto i = 0; i < 6; ++i) {
        if (!ode_napi_read_into(obj.Get(i).Unwrap(), parsed[i])) {
            Napi::TypeError::New(value.Env(), "Missing value for element").ThrowAsJavaScriptException();
            return false;
        }
    }
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const double(&source)[6]) {
    auto arr = Napi::Array::New(env, 6);
    arr.Set((uint32_t) 0, source[0]);
    arr.Set((uint32_t) 1, source[1]);
    arr.Set((uint32_t) 2, source[2]);
    arr.Set((uint32_t) 3, source[3]);
    arr.Set((uint32_t) 4, source[4]);
    arr.Set((uint32_t) 5, source[5]);
    return arr;
}
