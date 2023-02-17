
#include <ode/graphics/gl.h>

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

// Disable vertex array functions if it's not enabled in ODE.
//   ImGui uses their IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY define, however this is set within the imgui_impl_opengl3.cpp.
#ifndef ODE_GL_ENABLE_VERTEX_ARRAYS
#define glBindVertexArray
#define glGenVertexArrays
#define glDeleteVertexArrays
#endif

#include "imgui_impl_opengl3.cpp"
