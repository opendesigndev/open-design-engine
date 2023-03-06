
// FILE GENERATED BY generate-api-bindings.py
#include <string>
#include "addon.h"
#include "napi-wrap.h"
#include "gen-api-base.h"
#include "gen-renderer-api.h"
Napi::Value bind_ode_destroyBitmap(const Napi::CallbackInfo& info);
Napi::Value bind_ode_createRendererContext(const Napi::CallbackInfo& info);
Napi::Value bind_ode_destroyRendererContext(const Napi::CallbackInfo& info);
Napi::Value bind_ode_createDesignImageBase(const Napi::CallbackInfo& info);
Napi::Value bind_ode_destroyDesignImageBase(const Napi::CallbackInfo& info);
Napi::Value bind_ode_design_loadImagePixels(const Napi::CallbackInfo& info);
Napi::Value bind_ode_pr1_drawComponent(const Napi::CallbackInfo& info);
Napi::Value bind_ode_pr1_createAnimationRenderer(const Napi::CallbackInfo& info);
Napi::Value bind_ode_pr1_destroyAnimationRenderer(const Napi::CallbackInfo& info);
Napi::Value bind_ode_pr1_animation_drawFrame(const Napi::CallbackInfo& info);

Napi::Object init_gen_renderer_api(Napi::Env env, Napi::Object exports) {

    exports.Set("PIXEL_FORMAT_RGBA", uint32_t(ODE_PIXEL_FORMAT_RGBA));

    exports.Set("PIXEL_FORMAT_PREMULTIPLIED_RGBA", uint32_t(ODE_PIXEL_FORMAT_PREMULTIPLIED_RGBA));




    Handle<ODE_RendererContextHandle>::Export(exports);
    Handle<ODE_DesignImageBaseHandle>::Export(exports);
    Handle<ODE_PR1_AnimationRendererHandle>::Export(exports);
    exports.Set("destroyBitmap", Napi::Function::New<bind_ode_destroyBitmap>(env, "destroyBitmap"));
    exports.Set("createRendererContext", Napi::Function::New<bind_ode_createRendererContext>(env, "createRendererContext"));
    exports.Set("destroyRendererContext", Napi::Function::New<bind_ode_destroyRendererContext>(env, "destroyRendererContext"));
    exports.Set("createDesignImageBase", Napi::Function::New<bind_ode_createDesignImageBase>(env, "createDesignImageBase"));
    exports.Set("destroyDesignImageBase", Napi::Function::New<bind_ode_destroyDesignImageBase>(env, "destroyDesignImageBase"));
    exports.Set("design_loadImagePixels", Napi::Function::New<bind_ode_design_loadImagePixels>(env, "design_loadImagePixels"));
    exports.Set("pr1_drawComponent", Napi::Function::New<bind_ode_pr1_drawComponent>(env, "pr1_drawComponent"));
    exports.Set("pr1_createAnimationRenderer", Napi::Function::New<bind_ode_pr1_createAnimationRenderer>(env, "pr1_createAnimationRenderer"));
    exports.Set("pr1_destroyAnimationRenderer", Napi::Function::New<bind_ode_pr1_destroyAnimationRenderer>(env, "pr1_destroyAnimationRenderer"));
    exports.Set("pr1_animation_drawFrame", Napi::Function::New<bind_ode_pr1_animation_drawFrame>(env, "pr1_animation_drawFrame"));    return exports;
}

template<>
bool Autobind<ODE_Bitmap>::read_into(const Napi::Value& value, ODE_Bitmap& parsed){
    Napi::Env env = value.Env();
    Napi::Object obj = value.As<Napi::Object>();
    if(!Autobind<int>::read_into(obj.Get("format"), parsed.format)) {
        return false;
    }
    uintptr_t ptr_pixels;
    if(Autobind<uintptr_t>::read_into(obj.Get("pixels"), ptr_pixels)) {
        parsed.pixels = reinterpret_cast<ODE_VarDataPtr>(ptr_pixels);
    } else {
        return false;
    }
    if(!Autobind<int>::read_into(obj.Get("width"), parsed.width)) {
        return false;
    }
    if(!Autobind<int>::read_into(obj.Get("height"), parsed.height)) {
        return false;
    }
    return true;
}
template<>
void Autobind<ODE_Bitmap>::write_from(Napi::Value value, const ODE_Bitmap& parsed){
}

template<>
bool Autobind<ODE_BitmapRef>::read_into(const Napi::Value& value, ODE_BitmapRef& parsed){
    Napi::Env env = value.Env();
    Napi::Object obj = value.As<Napi::Object>();
    if(!Autobind<int>::read_into(obj.Get("format"), parsed.format)) {
        return false;
    }
    uintptr_t ptr_pixels;
    if(Autobind<uintptr_t>::read_into(obj.Get("pixels"), ptr_pixels)) {
        parsed.pixels = reinterpret_cast<ODE_ConstDataPtr>(ptr_pixels);
    } else {
        return false;
    }
    if(!Autobind<int>::read_into(obj.Get("width"), parsed.width)) {
        return false;
    }
    if(!Autobind<int>::read_into(obj.Get("height"), parsed.height)) {
        return false;
    }
    return true;
}
template<>
void Autobind<ODE_BitmapRef>::write_from(Napi::Value value, const ODE_BitmapRef& parsed){
}

template<>
bool Autobind<ODE_PR1_FrameView>::read_into(const Napi::Value& value, ODE_PR1_FrameView& parsed){
    Napi::Env env = value.Env();
    Napi::Object obj = value.As<Napi::Object>();
    if(!Autobind<int>::read_into(obj.Get("width"), parsed.width)) {
        return false;
    }
    if(!Autobind<int>::read_into(obj.Get("height"), parsed.height)) {
        return false;
    }
    if(!Autobind<ODE_Vector2>::read_into(obj.Get("offset"), parsed.offset)) {
        return false;
    }
    if(!Autobind<ODE_Scalar>::read_into(obj.Get("scale"), parsed.scale)) {
        return false;
    }
    return true;
}
template<>
void Autobind<ODE_PR1_FrameView>::write_from(Napi::Value value, const ODE_PR1_FrameView& parsed){
}

template<>
const char* Handle<ODE_RendererContextHandle>::name = "RendererContextHandle";
template<>
bool Autobind<ODE_RendererContextHandle>::read_into(const Napi::Value& value, ODE_RendererContextHandle& target) {
    auto optional = Handle<ODE_RendererContextHandle>::Read(value);
    if(optional) { target = *optional; return true; }
    return false;
}
template<>
void Autobind<ODE_RendererContextHandle>::write_from(Napi::Value value, const ODE_RendererContextHandle& handle){
    if(Handle<ODE_RendererContextHandle>::Write(value, handle)) { /* TODO: figure out error handling */ }
}

template<>
const char* Handle<ODE_DesignImageBaseHandle>::name = "DesignImageBaseHandle";
template<>
bool Autobind<ODE_DesignImageBaseHandle>::read_into(const Napi::Value& value, ODE_DesignImageBaseHandle& target) {
    auto optional = Handle<ODE_DesignImageBaseHandle>::Read(value);
    if(optional) { target = *optional; return true; }
    return false;
}
template<>
void Autobind<ODE_DesignImageBaseHandle>::write_from(Napi::Value value, const ODE_DesignImageBaseHandle& handle){
    if(Handle<ODE_DesignImageBaseHandle>::Write(value, handle)) { /* TODO: figure out error handling */ }
}

template<>
const char* Handle<ODE_PR1_AnimationRendererHandle>::name = "PR1_AnimationRendererHandle";
template<>
bool Autobind<ODE_PR1_AnimationRendererHandle>::read_into(const Napi::Value& value, ODE_PR1_AnimationRendererHandle& target) {
    auto optional = Handle<ODE_PR1_AnimationRendererHandle>::Read(value);
    if(optional) { target = *optional; return true; }
    return false;
}
template<>
void Autobind<ODE_PR1_AnimationRendererHandle>::write_from(Napi::Value value, const ODE_PR1_AnimationRendererHandle& handle){
    if(Handle<ODE_PR1_AnimationRendererHandle>::Write(value, handle)) { /* TODO: figure out error handling */ }
}

Napi::Value bind_ode_destroyBitmap(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_Bitmap v1;
    if(!Autobind<ODE_Bitmap>::read_into(info[0], v1)) return Napi::Value();
    auto result = ode_destroyBitmap(v1);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_createRendererContext(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_EngineHandle v1;
    if(!Autobind<ODE_EngineHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_RendererContextHandle rendererContext;
    ODE_StringRef v3;
    if(!Autobind<ODE_StringRef>::read_into(info[2], v3)) return Napi::Value();
    auto result = ode_createRendererContext(v1, &rendererContext, v3);
    Autobind<ODE_RendererContextHandle>::write_from(info[1], rendererContext);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_destroyRendererContext(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_RendererContextHandle v1;
    if(!Autobind<ODE_RendererContextHandle>::read_into(info[0], v1)) return Napi::Value();
    auto result = ode_destroyRendererContext(v1);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_createDesignImageBase(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_RendererContextHandle v1;
    if(!Autobind<ODE_RendererContextHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_DesignHandle v2;
    if(!Autobind<ODE_DesignHandle>::read_into(info[1], v2)) return Napi::Value();
    ODE_DesignImageBaseHandle designImageBase;
    auto result = ode_createDesignImageBase(v1, v2, &designImageBase);
    Autobind<ODE_DesignImageBaseHandle>::write_from(info[2], designImageBase);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_destroyDesignImageBase(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_DesignImageBaseHandle v1;
    if(!Autobind<ODE_DesignImageBaseHandle>::read_into(info[0], v1)) return Napi::Value();
    auto result = ode_destroyDesignImageBase(v1);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_design_loadImagePixels(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_DesignImageBaseHandle v1;
    if(!Autobind<ODE_DesignImageBaseHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_StringRef v2;
    if(!Autobind<ODE_StringRef>::read_into(info[1], v2)) return Napi::Value();
    ODE_BitmapRef v3;
    if(!Autobind<ODE_BitmapRef>::read_into(info[2], v3)) return Napi::Value();
    auto result = ode_design_loadImagePixels(v1, v2, v3);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_pr1_drawComponent(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_RendererContextHandle v1;
    if(!Autobind<ODE_RendererContextHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_ComponentHandle v2;
    if(!Autobind<ODE_ComponentHandle>::read_into(info[1], v2)) return Napi::Value();
    ODE_DesignImageBaseHandle v3;
    if(!Autobind<ODE_DesignImageBaseHandle>::read_into(info[2], v3)) return Napi::Value();
    ODE_Bitmap outputBitmap;
    ODE_PR1_FrameView frameView;
    if(!Autobind<ODE_PR1_FrameView>::read_into(info[4], frameView)) return Napi::Value();
    auto result = ode_pr1_drawComponent(v1, v2, v3, &outputBitmap, &frameView);
    Autobind<ODE_Bitmap>::write_from(info[3], outputBitmap);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_pr1_createAnimationRenderer(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_RendererContextHandle v1;
    if(!Autobind<ODE_RendererContextHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_ComponentHandle v2;
    if(!Autobind<ODE_ComponentHandle>::read_into(info[1], v2)) return Napi::Value();
    ODE_PR1_AnimationRendererHandle animationRenderer;
    ODE_DesignImageBaseHandle v4;
    if(!Autobind<ODE_DesignImageBaseHandle>::read_into(info[3], v4)) return Napi::Value();
    auto result = ode_pr1_createAnimationRenderer(v1, v2, &animationRenderer, v4);
    Autobind<ODE_PR1_AnimationRendererHandle>::write_from(info[2], animationRenderer);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_pr1_destroyAnimationRenderer(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_PR1_AnimationRendererHandle v1;
    if(!Autobind<ODE_PR1_AnimationRendererHandle>::read_into(info[0], v1)) return Napi::Value();
    auto result = ode_pr1_destroyAnimationRenderer(v1);
    return Napi::String::New(env, Result_to_string(result));
}

Napi::Value bind_ode_pr1_animation_drawFrame(const Napi::CallbackInfo& info) {
    auto env = info.Env();
    ODE_PR1_AnimationRendererHandle v1;
    if(!Autobind<ODE_PR1_AnimationRendererHandle>::read_into(info[0], v1)) return Napi::Value();
    ODE_PR1_FrameView frameView;
    if(!Autobind<ODE_PR1_FrameView>::read_into(info[1], frameView)) return Napi::Value();
    ODE_Scalar v3;
    if(!Autobind<ODE_Scalar>::read_into(info[2], v3)) return Napi::Value();
    auto result = ode_pr1_animation_drawFrame(v1, &frameView, v3);
    return Napi::String::New(env, Result_to_string(result));
}

