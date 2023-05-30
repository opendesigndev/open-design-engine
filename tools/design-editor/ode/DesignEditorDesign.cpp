
#include "DesignEditorDesign.h"

#include <ode/logic-api.h>
#include <ode/renderer-api.h>

#define CHECK(x) do { if ((x)) return -1; } while (false)

bool DesignEditorDesign::empty() const {
    return components.empty();
}

int DesignEditorDesign::create(ODE_EngineHandle engine, ODE_RendererContextHandle rc) {
    CHECK(ode_createDesign(engine, &design));
    CHECK(ode_createDesignImageBase(rc, design, &imageBase));
    components.clear();
    return 0;
}

int DesignEditorDesign::destroy() {
    CHECK(ode_destroyDesign(design));
    CHECK(ode_destroyDesignImageBase(imageBase));
    components.clear();
    return 0;
}
