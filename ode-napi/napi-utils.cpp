
#include "napi-utils.h"

#include <string>

namespace ode {
namespace napi {

std::string enumString(ODE_Result value);

bool checkResult(const Napi::Env &env, ODE_Result result) {
    if (result != ODE_RESULT_OK) {
        Napi::Error::New(env, enumString(result)).ThrowAsJavaScriptException();
        return false;
    }
    return true;
}

}
}
