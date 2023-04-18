
#include "DesignEditorContext.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <ode-graphics.h>

namespace {
#define CHECK(x) do { if ((x)) return -1; } while (false)
}

int DesignEditorContext::initialize() {
    ODE_EngineAttributes engineAttribs;
    CHECK(ode_initializeEngineAttributes(&engineAttribs));
    CHECK(ode_createEngine(&engine, &engineAttribs));
    CHECK(ode_createRendererContext(engine, &rc, ode_stringRef("Design Editor")));

    GLFWwindow *window = reinterpret_cast<ode::GraphicsContext *>(rc.ptr)->getNativeHandle<GLFWwindow *>();
    glfwGetFramebufferSize(window, &frameView.width, &frameView.height);
    frameView.scale = 1;

    CHECK(ode_createDesign(engine, &design.design));
    CHECK(ode_createDesignImageBase(rc, design.design, &design.imageBase));

    return 0;
}

int DesignEditorContext::destroy() {
    CHECK(ode_destroyDesignImageBase(design.imageBase));
    CHECK(ode_destroyRendererContext(rc));
    CHECK(ode_destroyDesign(design.design));
    CHECK(ode_destroyEngine(engine));

    return 0;
}
