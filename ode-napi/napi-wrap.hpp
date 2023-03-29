
#include "napi-wrap.h"

#include <cstdint>
#include <ode/api-base.h>
#include <ode/logic-api.h>

namespace ode {
namespace napi {

template <int>
class _PointerIntTypeWrapper { };

template <>
class _PointerIntTypeWrapper<32> {
public:
    typedef int32_t T;
};

template <>
class _PointerIntTypeWrapper<64> {
public:
    typedef int64_t T;
};

typedef _PointerIntTypeWrapper<(sizeof(void *) > sizeof(int32_t) ? 64 : 32)>::T PointerIntType;

inline Napi::Value wrap(const Napi::Env &env, int src) {
    return Napi::Number::New(env, src);
}

inline Napi::Value wrap(const Napi::Env &env, size_t src) {
    if (size_t(double(src)) != src) {
        Napi::Error::New(env, "Too large to convert to JS number").ThrowAsJavaScriptException();
        return Napi::Value();
    }
    return Napi::Number::New(env, double(src));
}

inline Napi::Value wrap(const Napi::Env &env, ODE_Scalar src) {
    return Napi::Number::New(env, src);
}

inline Napi::Value wrap(const Napi::Env &env, ODE_VarDataPtr src) {
    return wrap(env, size_t(reinterpret_cast<uintptr_t>(src)));
}

inline Napi::Value wrap(const Napi::Env &env, ODE_ConstDataPtr src) {
    return wrap(env, size_t(reinterpret_cast<uintptr_t>(src)));
}

inline bool unwrap(int &dst, const Napi::Value &src) {
    int32_t v = (int32_t) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = v;
    return true;
}

inline bool unwrap(size_t &dst, const Napi::Value &src) {
    PointerIntType v = (PointerIntType) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = size_t(v);
    return true;
}

inline bool unwrap(ODE_Scalar &dst, const Napi::Value &src) {
    dst = (ODE_Scalar) src.As<Napi::Number>();
    return true;
}

inline bool unwrap(ODE_VarDataPtr &dst, const Napi::Value &src) {
    intptr_t v = (intptr_t) (PointerIntType) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = reinterpret_cast<ODE_VarDataPtr>(v);
    return true;
}

inline bool unwrap(ODE_ConstDataPtr &dst, const Napi::Value &src) {
    intptr_t v = (intptr_t) (PointerIntType) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = reinterpret_cast<ODE_ConstDataPtr>(v);
    return true;
}

template <typename T>
inline Napi::Value wrap(const Napi::Env &env, T *src) {
    return wrap(env, size_t(reinterpret_cast<uintptr_t>(src)));
}

template <typename T>
inline Napi::Value wrap(const Napi::Env &env, const T *src) {
    return wrap(env, size_t(reinterpret_cast<uintptr_t>(src)));
}

template <typename T>
inline bool unwrap(T *&dst, const Napi::Value &src) {
    intptr_t v = (intptr_t) (PointerIntType) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = reinterpret_cast<T *>(v);
    return true;
}

template <typename T>
inline bool unwrap(const T *&dst, const Napi::Value &src) {
    intptr_t v = (intptr_t) (PointerIntType) src.As<Napi::Number>();
    if (src.Env().IsExceptionPending())
        return false;
    dst = reinterpret_cast<const T *>(v);
    return true;
}

template <typename T>
inline Napi::Value wrapHandle(const Napi::Env &env, const char *name, const T &src) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set(name, (uintptr_t) src.ptr);
    return obj;
}

template <typename T>
inline bool unwrapHandle(T &dst, const char *name, const Napi::Value &src) {
    Napi::Env env = src.Env();
    Napi::Object obj = src.As<Napi::Object>();
    if (env.IsExceptionPending())
        return false;
    Napi::MaybeOrValue<Napi::Value> maybe = obj.Get(name);
    if (env.IsExceptionPending())
        return false;
    if (maybe.IsNothing()) {
        Napi::Error::New(env, "Incorrect argument type (not a handle)").ThrowAsJavaScriptException();
        return false;
    }
    dst.ptr = reinterpret_cast<decltype(dst.ptr)>(reinterpret_cast<void *>((uintptr_t) (PointerIntType) maybe.Unwrap().As<Napi::Number>()));
    return true;
}

template <unsigned N, typename T>
inline Napi::Value wrapArray(const Napi::Env &env, const T src[N]) {
    Napi::Array arr = Napi::Array::New(env, N);
    for (uint32_t i = 0; i < N; ++i)
        arr.Set(i, wrap(env, src[i]));
    return arr;
}

template <unsigned N, typename T>
inline bool unwrapArray(T dst[N], const Napi::Value &src) {
    Napi::Array arr = src.As<Napi::Array>();
    for (uint32_t i = 0; i < N; ++i) {
        if (!unwrap(dst[i], arr.Get(i))) {
            Napi::TypeError::New(src.Env(), "Missing value for element").ThrowAsJavaScriptException();
            return false;
        }
    }
    return true;
}

template <typename T>
inline bool unwrap(T &dst, const Napi::Maybe<Napi::Value> &src) {
    if (src.IsNothing())
        return false;
    return unwrap(dst, src.Unwrap());
}

template <unsigned N, typename T>
inline bool unwrapArray(T dst[N], const Napi::Maybe<Napi::Value> &src) {
    if (src.IsNothing())
        return false;
    return unwrapArray<N>(dst, src.Unwrap());
}

inline bool copy(const Napi::Value &dst, const Napi::Value &src) {
    if (!(dst.IsObject() && src.IsObject()))
        return false;
    Napi::Object d = dst.As<Napi::Object>();
    Napi::Object s = src.As<Napi::Object>();
    Napi::Maybe<Napi::Array> maybeProps = s.GetPropertyNames();
    if (maybeProps.IsNothing())
        return false;
    Napi::Array props = maybeProps.Unwrap();
    uint32_t len = props.Length();
    for (uint32_t i = 0; i < len; ++i) {
        Napi::Value key = props.Get(i).Unwrap();
        d.Set(key, s.Get(key).Unwrap());
    }
    return true;
}

template <typename T>
inline bool copyWrapped(const Napi::Value &dst, const T &src) {
    return copy(dst, wrap(dst.Env(), src));
}

}
}
