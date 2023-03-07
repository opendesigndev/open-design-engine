#include "addon.h"
#include <ode/api-base.h>
#include <ode/logic-api.h>
#include "napi-wrap.h"
#include "gen-api-base.h"
#include <iostream>

Napi::Object init_logic_api(Napi::Env env, Napi::Object exports) {
    return exports;
}

template<>
bool Autobind<ODE_LayerType>::read_into(const Napi::Value &value, ODE_LayerType &parsed) {
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_LayerType>::read_into").ThrowAsJavaScriptException();
    return false;
}

template<>
bool Autobind<ODE_ParseError::Type>::read_into(const Napi::Value &value, ODE_ParseError::Type &parsed) {
    Napi::Error::New(value.Env(), "Not implemented: Autobind<ODE_ParseError::Type>::read_into").ThrowAsJavaScriptException();
    return false;
}
