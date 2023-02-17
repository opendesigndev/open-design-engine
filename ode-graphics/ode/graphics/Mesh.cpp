
#include "Mesh.h"

namespace ode {

GLint Mesh::maxVertexAttribs;

Mesh::Mesh() {
    #ifndef ODE_GL_ENABLE_VERTEX_ARRAYS
        totalAttrSize = 0;
    #endif
    glGenBuffers(1, &vbo);
    #ifdef ODE_GL_ENABLE_VERTEX_ARRAYS
        glGenVertexArrays(1, &vao);
    #endif
    ODE_CHECK_GL_ERROR();
    primitives = GL_INVALID_ENUM;
    vertexCount = 0;
    if (!maxVertexAttribs) {
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    }
}

Mesh::~Mesh() {
    #ifdef ODE_GL_ENABLE_VERTEX_ARRAYS
        glDeleteVertexArrays(1, &vao);
    #endif
    glDeleteBuffers(1, &vbo);
    ODE_CHECK_GL_ERROR();
    //LOG_OWN_ACTION(MESH_DELETION, 1, "");
}

bool Mesh::initialize(const float *data, const int *attributeSizes, int attributeCount, GLenum primitives, int vertexCount) {
    int totalAttributeSize = 0;
    for (int i = 0; i < attributeCount; ++i)
        totalAttributeSize += attributeSizes[i];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*totalAttributeSize*vertexCount, data, GL_STATIC_DRAW);
    #ifdef ODE_GL_ENABLE_VERTEX_ARRAYS
        glBindVertexArray(vao);
        int start = 0;
        for (int i = 0; i < attributeCount; ++i) {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, GL_FALSE, totalAttributeSize*sizeof(GLfloat), (const void *) (uintptr_t) start);
            start += attributeSizes[i]*sizeof(GLfloat);
        }
    #else
        attrSizes.assign(attributeSizes, attributeSizes+attributeCount);
        totalAttrSize = totalAttributeSize;
    #endif
    ODE_CHECK_GL_ERROR();
    this->primitives = primitives;
    this->vertexCount = vertexCount;
    //LOG_OWN_ACTION(MESH_CREATION, vertexCount, "");
    return true;
}

void Mesh::draw() const {
    #ifdef ODE_GL_ENABLE_VERTEX_ARRAYS
        glBindVertexArray(vao);
    #else
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        size_t start = 0;
        size_t i = 0;
        for (; i < attrSizes.size(); ++i) {
            glEnableVertexAttribArray((GLuint) i);
            glVertexAttribPointer((GLuint) i, attrSizes[i], GL_FLOAT, GL_FALSE, totalAttrSize*sizeof(GLfloat), (const void *) start);
            start += attrSizes[i]*sizeof(GLfloat);
        }
        // TODO optimize this
        for (; i < maxVertexAttribs; ++i) {
            glDisableVertexAttribArray((GLuint) i);
        }
    #endif
    ODE_CHECK_GL_ERROR();
    glDrawArrays(primitives, 0, vertexCount);
    ODE_CHECK_GL_ERROR();
    //LOG_OWN_ACTION(DRAW_CALL, vertexCount, "");
}

}
