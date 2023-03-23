
#pragma once

#include <ode/api-base.h>
#include <ode/logic-api.h>
#include "napi-base.h"

namespace ode {
namespace napi {

Napi::Value wrap(const Napi::Env &env, int src);
Napi::Value wrap(const Napi::Env &env, size_t src);
Napi::Value wrap(const Napi::Env &env, ODE_Scalar src);
Napi::Value wrap(const Napi::Env &env, ODE_VarDataPtr src);
Napi::Value wrap(const Napi::Env &env, ODE_ConstDataPtr src);

bool unwrap(int &dst, const Napi::Value &src);
bool unwrap(size_t &dst, const Napi::Value &src);
bool unwrap(ODE_Scalar &dst, const Napi::Value &src);
bool unwrap(ODE_VarDataPtr &dst, const Napi::Value &src);
bool unwrap(ODE_ConstDataPtr &dst, const Napi::Value &src);

template <typename T>
Napi::Value wrap(const Napi::Env &env, T *src);
template <typename T>
Napi::Value wrap(const Napi::Env &env, const T *src);

template <typename T>
bool unwrap(T *&dst, const Napi::Value &src);
template <typename T>
bool unwrap(const T *&dst, const Napi::Value &src);

template <typename T>
Napi::Value wrapHandle(const Napi::Env &env, const char *name, const T &src);

template <typename T>
bool unwrapHandle(T &dst, const char *name, const Napi::Value &src);

Napi::Value wrap(const Napi::Env &env, const ODE_MemoryRef &src);
bool unwrap(ODE_MemoryRef &dst, const Napi::Value &src);

Napi::Value wrap(const Napi::Env &env, const ODE_Transformation &src);
bool unwrap(ODE_Transformation &dst, const Napi::Value &src);

template <typename T>
bool unwrap(T &dst, const Napi::Maybe<Napi::Value> &src);

bool copy(const Napi::Value &dst, const Napi::Value &src);

template <typename T>
bool copyWrapped(const Napi::Value &dst, const T &src);

}
}

#include "napi-wrap.hpp"
