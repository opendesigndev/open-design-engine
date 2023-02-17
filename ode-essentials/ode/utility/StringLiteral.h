
#pragma once

namespace ode {

struct StringLiteral {
    const char *string;
    int length;

    constexpr StringLiteral() : string(""), length(0) { }
    constexpr StringLiteral(const char *str, int len) : string(str), length(len) { }
};

}

#define ODE_STRLIT(str) ::ode::StringLiteral(("" str), int(sizeof(str)-1))
