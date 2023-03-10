#pragma once
#include "addon.h"

Napi::Object init_api_base(Napi::Env env, Napi::Object exports);
Napi::Value ode_napi_serialize(Napi::Env env, const int &value);
Napi::Value ode_napi_serialize(Napi::Env env, const unsigned long &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Vector2 &parsed);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Rectangle &parsed);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_ConstDataPtr &parsed);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_VarDataPtr &parsed);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Scalar &parsed);
Napi::Value ode_napi_serialize(Napi::Env env, const double(&source)[6]);
