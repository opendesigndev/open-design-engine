
#include <ode-graphics.h>

using namespace ode;

int basicRenderingOutput(GraphicsContext &gc);

int main() {
    GraphicsContext gc(GraphicsContext::OFFSCREEN);

    if (int error = basicRenderingOutput(gc))
        return error;

    return 0;
}
