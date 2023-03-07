
#include "text-renderer-instance.h"

#include <cstdio>

namespace ode {

static odtr::ContextOptions contextOptions() {
    odtr::ContextOptions options;
    options.errorFunc = [](const std::string &message) {
        fprintf(stderr, "Text Renderer error: %s\n", message.c_str());
    };
    options.warnFunc = [](const std::string &message) {
        fprintf(stderr, "Text Renderer warning: %s\n", message.c_str());
    };
    options.infoFunc = [](const std::string &) { };
    return options;
}

const odtr::ContextHandle TEXT_RENDERER_CONTEXT = odtr::createContext(contextOptions());

class TextRendererDestroyer {
public:
    ~TextRendererDestroyer() {
        if (TEXT_RENDERER_CONTEXT)
            odtr::destroyContext(TEXT_RENDERER_CONTEXT);
    }
} textRendererDestroyer;

}
