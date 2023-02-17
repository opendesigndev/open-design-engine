
// Dummy functions to resolve Skia link errors

#include <GL/glx.h>

void (*glXGetProcAddress(const GLubyte *))() {
    return nullptr;
}

GLXContext glXGetCurrentContext() {
    return nullptr;
}
