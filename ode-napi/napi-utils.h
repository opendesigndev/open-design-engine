
#pragma once

#include <ode/api-base.h>
#include "napi-base.h"

namespace ode {
namespace napi {

bool checkResult(const Napi::Env &env, ODE_Result result);

}
}
