
#include <string>
#include <ode/api-base.h>
#include "napi-base.h"

// Generated headers
#include <napi-api-base.h>
#include <napi-logic-api.h>
#include <napi-renderer-api.h>

namespace ode {
namespace napi {

Napi::Value makeString(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::String napiString = info[0].As<Napi::String>();
    if (env.IsExceptionPending())
        return Napi::Value();
    ODE_String string = ode_makeString((std::string) napiString);
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("data", Napi::Number::New(env, reinterpret_cast<uintptr_t>(string.data)));
    obj.Set("length", Napi::Number::New(env, string.length));
    return obj;
}

Napi::Value getString(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    ODE_StringRef ref;
    if (!unwrap(ref, info[0])) {
        Napi::Error error = env.GetAndClearPendingException();
        Napi::Error::New(env, "Failed to parse argument string ("+error.Message()+")").ThrowAsJavaScriptException();
        return Napi::Value();
    }
    return Napi::String::New(env, reinterpret_cast<const char *>(ref.data), ref.length);
}

Napi::Value makeMemoryBuffer(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::ArrayBuffer napiArrayBuffer = info[0].As<Napi::ArrayBuffer>();
    if (env.IsExceptionPending())
        return Napi::Value();
    ODE_MemoryBuffer buffer = ode_makeMemoryBuffer(napiArrayBuffer.Data(), napiArrayBuffer.ByteLength());
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("data", Napi::Number::New(env, reinterpret_cast<uintptr_t>(buffer.data)));
    obj.Set("length", Napi::Number::New(env, buffer.length));
    return obj;
}

Napi::Object initialize(Napi::Env env, Napi::Object exports) {
    exports.Set("makeString", Napi::Function::New<makeString>(env, "makeString"));
    exports.Set("getString", Napi::Function::New<getString>(env, "getString"));
    exports.Set("makeMemoryBuffer", Napi::Function::New<makeMemoryBuffer>(env, "makeMemoryBuffer"));
    api_base_export(env, exports);
    logic_api_export(env, exports);
    renderer_api_export(env, exports);
    return exports;
}

NODE_API_MODULE(ode, initialize);

}
}
