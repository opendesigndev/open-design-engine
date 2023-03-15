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
bool ode_napi_read_into(const Napi::Value &value, int &parsed);
bool ode_napi_read_into(const Napi::Value &value, unsigned long &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_Vector2 &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_ConstDataPtr &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_Rectangle &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_Scalar &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_VarDataPtr &parsed);
bool ode_napi_read_into(const Napi::Value &value, double(&parsed)[6]);
