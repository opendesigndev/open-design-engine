
#include "GraphicsContext.h"

#ifdef ODE_GRAPHICS_GLFW_CONTEXT

#include <cstdio>
#include "gl.h"
#include <GLFW/glfw3.h>
#include <ode/utils.h>
#include "FrameBuffer.h"

//#define ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE 1024

namespace ode {

class GraphicsContext::Internal {
public:
    GLFWwindow* window;
    GLuint screenFBO;
    bool ready;
};

int GraphicsContext::maxTextureSize = 0;
int GraphicsContext::maxRenderbufferSize = 0;

int GraphicsContext::getMaxTextureSize() {
    return maxTextureSize;
}

int GraphicsContext::getMaxRenderBufferSize() {
    return maxRenderbufferSize;
}

GraphicsContext::GraphicsContext(const Vector2i &dimensions) {
    initialize("Open Design Engine", dimensions, false);
}

GraphicsContext::GraphicsContext(const char *title, const Vector2i &dimensions) {
    initialize(title, dimensions, false);
}

GraphicsContext::GraphicsContext(Offscreen) {
    initialize("", defaultDimensions, true);
}

GraphicsContext::GraphicsContext(Offscreen, const Vector2i &dimensions) {
    initialize("", dimensions, true);
}

#ifdef ODE_DEBUG
static void APIENTRY reportGlMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    switch (id) {
        case 8: // API_ID_REDUNDANT_FBO performance warning has been generated. Redundant state change in glBindFramebuffer API call, FBO 4, "", already bound.
        case 131169: // Framebuffer detailed info: The driver allocated storage for renderbuffer.
        case 131185: // Buffer detailed info: Buffer object will use VIDEO memory as the source for buffer object operations. (ImGui)
        case 131204: // Texture state usage warning: Texture 0 is base level inconsistent. Check texture size. (To be investigated further, seems related to ImGui)
            return;
    }
    const char *severityStr = "?";
    switch (severity){
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityStr = "i";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityStr = ".";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityStr = ":";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            severityStr = "!";
            break;
    }
    const char *sourceStr = "(unknown source)";
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "OTHER";
            break;
    }
    const char *typeStr = "(unknown type)";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "OTHER";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "MARKER";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "PUSH_GROUP";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "POP_GROUP";
            break;
    }
    fprintf(stderr, "{GL}: %s %s %s %u %s\n\n", severityStr, sourceStr, typeStr, id, message);
}

static void glfwErrorCallback(int error, const char *description) {
    fprintf(stderr, "[ODE : GraphicsContext-GLFW] Glfw Error %d: %s\n", error, description);
}
#endif

void GraphicsContext::initialize(const char *title, const Vector2i &dimensions, bool offscreen) {
    data.reset(new Internal { });
    #ifdef __APPLE__
        glfwInitHint(GLFW_COCOA_MENUBAR, offscreen ? GLFW_FALSE : GLFW_TRUE);
    #endif
    #ifdef ODE_DEBUG
        glfwSetErrorCallback(glfwErrorCallback);
    #endif

    if (glfwInit() != GLFW_TRUE)
        return;

    #ifdef ODE_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, offscreen ? GLFW_FALSE : GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, offscreen ? GLFW_FALSE : GLFW_TRUE);

    #ifdef ODE_GRAPHICS_OPENGL3_CORE
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        #endif
    #endif

    if (!(data->window = glfwCreateWindow(dimensions.x, dimensions.y, title, nullptr, nullptr)))
        return;
    glfwMakeContextCurrent(data->window);

    if (glewInit() != GLEW_OK)
        return;

    #ifdef ODE_DEBUG
        if (glDebugMessageCallback && glDebugMessageControl) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(reportGlMessage, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    #endif

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (!maxTextureSize) {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
        #if defined(ODE_DEBUG) && defined(ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE)
            maxTextureSize = ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE;
        #endif
        //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "GL_MAX_TEXTURE_SIZE = %d", maxTextureSize);
    }
    if (!maxRenderbufferSize) {
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
        #if defined(ODE_DEBUG) && defined(ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE)
            maxRenderbufferSize = ODE_GRAPHICS_EXPLICIT_MAX_IMAGE_SIZE;
        #endif
        //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "GL_MAX_RENDERBUFFER_SIZE = %d", maxRenderbufferSize);
    }

    //LOG_OWN_ACTION(GRAPHICS_CONTEXT_CONSTRUCTION, width*height, "");

    // Enable vertical sync
    if (!offscreen)
        glfwSwapInterval(1);

    //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "OpenGL profile: %s", glfwGetWindowAttrib(data_->window, GLFW_OPENGL_PROFILE) == GLFW_OPENGL_CORE_PROFILE ? "core" : "compatibility");
    //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "OpenGL version: %s", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    //Log::instance.logf(Log::CORE_OPENGL, Log::INFORMATION, "GPU: %s %s", reinterpret_cast<const char*>(glGetString(GL_VENDOR)), reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    ODE_CHECK_GL_ERROR();
    data->ready = true;
}

GraphicsContext::~GraphicsContext() {
    if (data->window)
        glfwDestroyWindow(data->window);
    glfwTerminate();
    //LOG_OWN_ACTION(GRAPHICS_CONTEXT_DESTRUCTION, 1, "");
}

GraphicsContext::operator bool() const {
    return data->ready;
}

Vector2i GraphicsContext::dimensions() const {
    Vector2i result;
    if (data->window)
        glfwGetFramebufferSize(data->window, &result.x, &result.y);
    return result;
}

void GraphicsContext::swapOutputFramebuffer() {
    glfwSwapBuffers(data->window);
}

void GraphicsContext::bindOutputFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, data->screenFBO);
}

void GraphicsContext::spoofOutputFramebuffer(FrameBuffer *fb) {
    data->screenFBO = fb ? fb->handle : 0;
}

template <>
GLFWwindow* GraphicsContext::getNativeHandle() {
    return data->window;
}

}

#endif // ODE_GRAPHICS_GLFW_CONTEXT
