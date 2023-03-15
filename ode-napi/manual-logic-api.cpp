#include "addon.h"

bool ode_napi_read_into(const Napi::Value &value, ODE_Transformation &target) {
    Napi::Array obj = value.As<Napi::Array>();
    for (auto i = 0; i < 6; ++i) {
        if (!ode_napi_read_into(obj.Get(i).Unwrap(), target.matrix[i])) {
            Napi::TypeError::New(value.Env(), "Missing value for element").ThrowAsJavaScriptException();
            return false;
        }
    }
    return true;
}
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Transformation &value) {
    auto arr = Napi::Array::New(env, 6);
    arr.Set((uint32_t) 0, value.matrix[0]);
    arr.Set((uint32_t) 1, value.matrix[1]);
    arr.Set((uint32_t) 2, value.matrix[2]);
    arr.Set((uint32_t) 3, value.matrix[3]);
    arr.Set((uint32_t) 4, value.matrix[4]);
    arr.Set((uint32_t) 5, value.matrix[5]);
    return arr;
}
