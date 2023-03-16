#include "addon.h"
#include "gen-api-base.h"


Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports = init_gen_api_base(env, exports);
    exports = init_gen_logic_api(env, exports);
    exports = init_gen_renderer_api(env, exports);
    exports = init_api_base(env, exports);
    return exports;
}

NODE_API_MODULE(ode, Init)

bool check_result(Napi::Env env, ODE_Result result) {
    if (result != ODE_RESULT_OK) {
        Napi::Error::New(env, ode_napi_enum_to_string(result)).ThrowAsJavaScriptException();
        return false;
    }
    return true;
}

bool copy_values(Napi::Value from, Napi::Value to) {
    if (!from.IsObject() || !to.IsObject()) return false;
    Napi::Object f = from.As<Napi::Object>();
    Napi::Object t = to.As<Napi::Object>();
    Napi::Maybe<Napi::Array> mprops = f.GetPropertyNames();
    if (mprops.IsNothing()) return false;
    Napi::Array props = mprops.Unwrap();
    uint32_t len = props.Length();
    for (uint32_t i = 0; i < len; ++i) {
        Napi::Value key = props.Get(i).Unwrap();
        t.Set(key, f.Get(key).Unwrap());
    }
    return true;
}
