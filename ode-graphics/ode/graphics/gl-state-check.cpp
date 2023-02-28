
#include "gl-state-check.h"

#include <cstdio>
#include "gl.h"

namespace ode {

bool checkGlState() {
    bool ok = true;

    #define REQUIRE_EQUAL(name, ref, x, y) ODE_STATEMENT( \
        if ((x) != (y)) { \
            fprintf(stderr, "Bad GL state: %s != %s\n", name, ref); \
            ok = false; \
        } \
    )
    #define REQUIRE_ENABLEMENT(name, value) ODE_STATEMENT( \
        GLboolean x = glIsEnabled(name); \
        REQUIRE_EQUAL(#name, #value, x, value); \
    )
    #define REQUIRE_INT(name, value) ODE_STATEMENT( \
        GLint x = !(value); \
        glGetIntegerv((name), &x); \
        REQUIRE_EQUAL(#name, #value, x, value); \
    )
    #define REQUIRE_INT2(name, v0, v1) ODE_STATEMENT( \
        GLint x[2] = { !(v0), !(v1) }; \
        glGetIntegerv((name), x); \
        if (!(x[0] == (v0) && x[1] == (v1))) { \
            fprintf(stderr, "Bad GL state: %s != %s, %s\n", #name, #v0, #v1); \
            ok = false; \
        } \
    )
    #define REQUIRE_INT4(name, v0, v1, v2, v3) ODE_STATEMENT( \
        GLint x[4] = { !(v0), !(v1), !(v2), !(v3) }; \
        glGetIntegerv((name), x); \
        if (!(x[0] == (v0) && x[1] == (v1) && x[2] == (v2) && x[3] == (v3))) { \
            fprintf(stderr, "Bad GL state: %s != %s, %s, %s, %s\n", #name, #v0, #v1, #v2, #v3); \
            ok = false; \
        } \
    )

    GLint maxTextureUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    if (maxTextureUnits < 4) {
        fprintf(stderr, "Insufficient GL_MAX_TEXTURE_IMAGE_UNITS == %d\n", (int) maxTextureUnits);
        ok = false;
    }
    // Check that no sampler objects are bound (they override texture parameters)
    #ifdef GL_SAMPLER_BINDING
        if (glBindSampler) {
            for (GLint i = maxTextureUnits; i--;) {
                glActiveTexture(GL_TEXTURE0+i);
                GLint sampler = 0;
                glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
                if (sampler) {
                    fprintf(stderr, "Bad GL state: sampler bound to texture unit %d\n", (int) i);
                    ok = false;
                }
            }
        }
    #endif

    REQUIRE_ENABLEMENT(GL_BLEND, GL_FALSE);
    REQUIRE_ENABLEMENT(GL_CULL_FACE, GL_FALSE);
    REQUIRE_ENABLEMENT(GL_DEPTH_TEST, GL_FALSE);
    REQUIRE_ENABLEMENT(GL_STENCIL_TEST, GL_FALSE);
    REQUIRE_ENABLEMENT(GL_SCISSOR_TEST, GL_FALSE);

    #ifdef GL_POLYGON_MODE
        REQUIRE_INT2(GL_POLYGON_MODE, GL_FILL, GL_FILL);
    #endif
    REQUIRE_INT4(GL_COLOR_WRITEMASK, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    REQUIRE_INT(GL_DEPTH_WRITEMASK, GL_FALSE);
    REQUIRE_INT(GL_BLEND_EQUATION_RGB, GL_FUNC_ADD);
    REQUIRE_INT(GL_BLEND_EQUATION_ALPHA, GL_FUNC_ADD);
    REQUIRE_INT(GL_PACK_ALIGNMENT, 1);
    REQUIRE_INT(GL_UNPACK_ALIGNMENT, 1);

    return ok;
}

}
