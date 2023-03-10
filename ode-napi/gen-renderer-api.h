
// FILE GENERATED BY generate-api-bindings.py
#pragma once
#include <napi.h>
#include <ode/renderer-api.h>

Napi::Value ode_napi_serialize(Napi::Env env, const ODE_Bitmap& source);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_BitmapRef& source);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_PR1_FrameView& source);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_RendererContextHandle& source);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_DesignImageBaseHandle& source);
Napi::Value ode_napi_serialize(Napi::Env env, const ODE_PR1_AnimationRendererHandle& source);
