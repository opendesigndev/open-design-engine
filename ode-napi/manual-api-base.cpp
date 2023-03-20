
#include "addon.h"

#include <cstddef>
#include <cstdint>
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
    int32_t v = (int32_t) value.As<Napi::Number>();
    if (value.Env().IsExceptionPending())
        return false;
    parsed = v;
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const int &value) {
    return Napi::Number::New(env, value);
}


bool ode_napi_read_into(const Napi::Value &value, size_t &parsed) {
    int64_t v = (int64_t) value.As<Napi::Number>();
    if (value.Env().IsExceptionPending())
        return false;
    parsed = v;
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const size_t &value) {
    ptrdiff_t signedValue = ptrdiff_t(value);
    if (uint32_t(value) != value || signedValue < 0) {
        Napi::Error::New(env, "Too big to convert to js number").ThrowAsJavaScriptException();
        return Napi::Value();
    }
    return Napi::Number::New(env, (uint32_t) value);
}

bool ode_napi_read_into(const Napi::Value &value, ODE_VarDataPtr &parsed) {
    intptr_t v = (intptr_t) (int64_t) value.As<Napi::Number>();
    if (value.Env().IsExceptionPending())
        return false;
    parsed = reinterpret_cast<ODE_VarDataPtr>(v);
    return true;
}

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_VarDataPtr &value) {
    return ode_napi_serialize(env, size_t(reinterpret_cast<uintptr_t>(value)));
}


Napi::Value ode_napi_serialize(Napi::Env env, const ODE_ConstDataPtr &value) {
    return ode_napi_serialize(env, size_t(reinterpret_cast<uintptr_t>(value)));
}

bool ode_napi_read_into(const Napi::Value &value, ODE_Scalar &parsed) {
    parsed = (ODE_Scalar) value.As<Napi::Number>();
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Scalar &parsed) {
    return Napi::Number::New(env, parsed);
}


Napi::Value node_napi_makeString(const Napi::CallbackInfo &info) {
    auto env = info.Env();
    auto napiString = info[0].As<Napi::String>();
    if (env.IsExceptionPending()) return Napi::Value();
    auto string = ode_makeString(napiString);
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

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_MemoryRef &value) {
    Napi::TypeError::New(env, "Not implemented yet").ThrowAsJavaScriptException();
    return Napi::Value();
}
bool ode_napi_read_into(const Napi::Value &value, ODE_MemoryRef &target) {
    if (!value.IsArrayBuffer()) {
        Napi::TypeError::New(value.Env(), "Expected ArrayBuffer").ThrowAsJavaScriptException();
        return false;
    }
    Napi::ArrayBuffer arrayBuffer = value.As<Napi::ArrayBuffer>();
    if (value.Env().IsExceptionPending()) return false;
    target.data = arrayBuffer.Data();
    target.length = arrayBuffer.ByteLength();
    return true;
}
