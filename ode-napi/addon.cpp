#include "addon.h"
#include "gen-api-base.h"

Napi::Value Method(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Addon &addon = Addon::from_env(env);
    return Napi::Number::New(env, (double)addon.new_counter);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Addon* addon = new Addon(exports);
    env.SetInstanceData(addon);
    
    exports.Set("readHandleCount", Napi::Function::New<Method>(env, "readHandleCount"));
    exports = init_gen_api_base(env, exports);
    exports = init_gen_logic_api(env, exports);
    exports = init_gen_renderer_api(env, exports);
    exports = init_api_base(env, exports);
    exports = init_logic_api(env, exports);
    return exports;
}

NODE_API_MODULE(ode, Init)

bool check_result(Napi::Env env, ODE_Result result) {
    if(result != ODE_RESULT_OK) {
        Napi::Error::New(env, Result_to_string(result)).ThrowAsJavaScriptException();
        return true;
    }
    return false;
}

Addon::Addon(Napi::Object exports) {
    this->exports = Napi::Persistent(exports);
    this->engineSymbol = Napi::Persistent(Napi::Symbol::New(exports.Env(), "EngineSymbol"));
    this->ptrSymbol = Napi::Persistent(Napi::Symbol::New(exports.Env(), "PtrSymbol"));
}

Addon &Addon::from_env(const Napi::Env& env) {
    auto ptr = env.GetInstanceData<Addon>();
    return *ptr;
}


template<>
bool Autobind<double [6]>::read_into(const Napi::Value& value, double (&parsed) [6]){
    Napi::Array obj = value.As<Napi::Array>();
    for(auto i = 0; i < 6; ++i) {
        if(!Autobind<double>::read_into(obj.Get(i).Unwrap(), parsed[i])) {
            Napi::TypeError::New(value.Env(), "Missing value for element").ThrowAsJavaScriptException();
            return false;
        }
    }
    return true;
}
