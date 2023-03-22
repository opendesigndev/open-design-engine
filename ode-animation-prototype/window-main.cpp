
#include <cstdio>
#include <chrono>
#include <cmath>
#include <ode-essentials.h>
#include <ode-graphics.h>
#include <ode/image/ImageBase.h>
#include <ode/optimized-renderer/Renderer.h>
#include <ode/renderer-api.h>
#include <GLFW/glfw3.h>

namespace ode {
extern int DEBUG_TEXTURES_CREATED;
}

using namespace ode;

static FilePath animationPath;
static ODE_ComponentHandle component;

// TODO this duplicates renderer-api.cpp
struct ODE_internal_RendererContext {
    GraphicsContext gc;
    std::unique_ptr<Renderer> renderer;

    inline ODE_internal_RendererContext(const char *label, const Vector2i &dimensions) : gc(label, dimensions) { }
    inline ODE_internal_RendererContext(GraphicsContext::Offscreen offscreen, const Vector2i &dimensions) : gc(offscreen, dimensions) { }
};

void onKeyPress(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F5 && !animationPath.empty()) {
        std::string animationDefJson;
        if (readFile(animationPath, animationDefJson)) {
            if (!animationDefJson.empty())
                ode_pr1_component_loadAnimation(component, ode_stringRef(animationDefJson), nullptr);
        }
    }
}

#define CHECK(x) do { if ((x)) return -1; } while (false)

int main(int argc, const char *const *argv) {

    double repeatPeriod = 10;

    if (argc < 2) {
        puts("Usage: ode-animation-prototype-window octopus.json animation-def.json");
        return 0;
    }

    std::string octopusJson, animationDefJson;
    if (!readFile(argv[1], octopusJson)) {
        fprintf(stderr, "Failed to read Octopus file \"%s\"\n", argv[1]);
        return 1;
    }
    if (argc > 2 && !readFile(animationPath = argv[2], animationDefJson)) {
        fprintf(stderr, "Failed to read animation definition file \"%s\"\n", argv[2]);
        return 1;
    }

    ODE_EngineHandle engine;
    ODE_EngineAttributes engineAttribs;
    CHECK(ode_initializeEngineAttributes(&engineAttribs));
    CHECK(ode_createEngine(&engine, &engineAttribs));

    ODE_DesignHandle design;
    CHECK(ode_createDesign(engine, &design));

    ODE_ComponentMetadata metadata = { };
    //ODE_ComponentHandle component; // global
    CHECK(ode_design_addComponentFromOctopusString(design, &component, metadata, ode_stringRef(octopusJson), nullptr));

    if (!animationDefJson.empty())
        CHECK(ode_pr1_component_loadAnimation(component, ode_stringRef(animationDefJson), nullptr));

    ODE_RendererContextHandle rc;
    CHECK(ode_createRendererContext(engine, &rc, ode_stringRef("Squid")));

    ODE_DesignImageBaseHandle imageBase;
    CHECK(ode_createDesignImageBase(rc, design, &imageBase));
    reinterpret_cast<ImageBase *>(imageBase.ptr)->setImageDirectory(FilePath(argv[1]).parent()); // TODO

    ODE_PR1_AnimationRendererHandle renderer;
    CHECK(ode_pr1_createAnimationRenderer(rc, component, &renderer, imageBase));


    GLFWwindow *window = rc.ptr->gc.getNativeHandle<GLFWwindow *>();
    glfwSetKeyCallback(window, &onKeyPress);

    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    int prevTexturesCreated = 0;
    int frameNo = 0;
    int printouts = 16;
    while (!glfwWindowShouldClose(window)) {
        double t = std::chrono::duration<double>(std::chrono::steady_clock::now()-startTime).count();
        if (repeatPeriod)
            t = fmod(t, repeatPeriod);
        ODE_PR1_FrameView frameView = { };
        glfwPollEvents();
        glfwGetFramebufferSize(window, &frameView.width, &frameView.height);
        frameView.scale = 1;
        ode_pr1_animation_drawFrame(renderer, &frameView, t);
        if (!frameNo++)
            printf("Textures created in first frame: %d\n", prevTexturesCreated = DEBUG_TEXTURES_CREATED);
        else if (DEBUG_TEXTURES_CREATED != prevTexturesCreated && printouts) {
            printf("!!! Additional %d textures created in frame %d\n", DEBUG_TEXTURES_CREATED-prevTexturesCreated, frameNo);
            prevTexturesCreated = DEBUG_TEXTURES_CREATED;
            --printouts;
        }
        rc.ptr->gc.swapOutputFramebuffer();
    }


    CHECK(ode_pr1_destroyAnimationRenderer(renderer));
    CHECK(ode_destroyDesignImageBase(imageBase));
    CHECK(ode_destroyRendererContext(rc));
    CHECK(ode_destroyDesign(design));
    CHECK(ode_destroyEngine(engine));

    return 0;
}
