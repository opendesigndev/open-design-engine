#pragma once
#include "addon.h"

Napi::Object init_api_base(Napi::Env env, Napi::Object exports);
Napi::Value ode_napi_serialize(Napi::Env env, const int &value);
Napi::Value ode_napi_serialize(Napi::Env env, const unsigned long &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_ConstDataPtr &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_VarDataPtr &value);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Scalar &value);
bool ode_napi_read_into(const Napi::Value &value, int &parsed);
bool ode_napi_read_into(const Napi::Value &value, unsigned long &parsed);
bool ode_napi_read_into(const Napi::Value &value, ODE_Scalar &parsed);

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_MemoryBuffer &value);
bool ode_napi_read_into(const Napi::Value &value, ODE_MemoryBuffer &target);
