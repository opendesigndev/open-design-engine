#pragma once
#include "addon.h"
#include <ode/logic-api.h>

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Transformation &value);
bool ode_napi_read_into(const Napi::Value &value, ODE_Transformation &target);
