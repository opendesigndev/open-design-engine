
#pragma once

#include <vector>
#include "gl.h"

namespace ode {

/**
 * The mesh class aggregates a vertex buffer object, optionally an index buffer object,
 * a vertex array object, the primitive type, and vertex count, which together describe
 * the geometry of a drawable "mesh".
 */
class Mesh {
public:
    Mesh();
    Mesh(const Mesh &) = delete;
    ~Mesh();
    Mesh &operator=(const Mesh &) = delete;
    bool initialize(const float *data, const int *attributeSizes, int attributeCount, GLenum primitives, int vertexCount);
    void draw() const;
    void drawPart(int start, int count) const;

private:
    static GLint maxVertexAttribs;

#ifdef ODE_GL_ENABLE_VERTEX_ARRAYS
    GLuint vao;
#else
    std::vector<int> attrSizes;
    int totalAttrSize;
#endif
    GLuint vbo;
    GLenum primitives;
    int vertexCount;

};

}
