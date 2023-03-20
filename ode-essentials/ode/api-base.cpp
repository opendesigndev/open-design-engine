
#include "api-base.h"

#include <cstdlib>
#include "utils.h"

ODE_Result ODE_API ode_destroyString(ODE_String string) {
    free(string.data);
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_allocateMemoryBuffer(ODE_MemoryBuffer *buffer, size_t length) {
    ODE_ASSERT(buffer);
    buffer->data = ODE_VarDataPtr();
    buffer->length = 0;
    return ode_reallocateMemoryBuffer(buffer, length);
}

ODE_Result ODE_API ode_reallocateMemoryBuffer(ODE_MemoryBuffer *buffer, size_t length) {
    ODE_ASSERT(buffer);
    if (void *data = realloc(reinterpret_cast<void *>(buffer->data), length)) {
        buffer->data = reinterpret_cast<ODE_VarDataPtr>(data);
        buffer->length = length;
        return ODE_RESULT_OK;
    }
    return ODE_RESULT_MEMORY_ALLOCATION_ERROR;
}

ODE_Result ODE_API ode_destroyMemoryBuffer(ODE_MemoryBuffer *buffer) {
    ODE_ASSERT(buffer);
    free(reinterpret_cast<void *>(buffer->data));
    buffer->data = ODE_VarDataPtr();
    buffer->length = 0;
    return ODE_RESULT_OK;
}

ODE_Result ODE_API ode_makeMemoryBuffer(ODE_MemoryRef ref, ODE_OUT_RETURN ODE_MemoryBuffer *buffer) {
    ODE_ASSERT(ref.data && ref.length > 0);
    auto result = ode_allocateMemoryBuffer(buffer, ref.length);
    if (result != ODE_RESULT_OK) return result;
    memcpy(buffer->data, ref.data, ref.length);
    return ODE_RESULT_OK;
}
