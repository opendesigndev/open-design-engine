
#include "textify-instance.h"

#include <cstdio>

namespace ode {

static textify::ContextOptions textifyContextOptions() {
    textify::ContextOptions options;
    options.errorFunc = [](const std::string &message) {
        fprintf(stderr, "Textify error: %s\n", message.c_str());
    };
    options.warnFunc = [](const std::string &message) {
        fprintf(stderr, "Textify warning: %s\n", message.c_str());
    };
    options.infoFunc = [](const std::string &) { };
    return options;
}

const textify::ContextHandle TEXTIFY_CONTEXT = textify::createContext(textifyContextOptions());

class TextifyDestroyer {
public:
    ~TextifyDestroyer() {
        if (TEXTIFY_CONTEXT)
            textify::destroyContext(TEXTIFY_CONTEXT);
    }
} textifyDestroyer;

}
