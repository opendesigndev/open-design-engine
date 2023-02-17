
#pragma once

#include <textify/textify-api.h>
#include <ode-essentials.h>
#include "../core/bounds.h"
#include "textify-instance.h"
#include "FontBase.h"

namespace ode {

class TextShapeHolder {

    textify::TextShapeHandle handle;

public:
    inline TextShapeHolder(textify::TextShapeHandle handle = nullptr) : handle(handle) { }

    TextShapeHolder(const TextShapeHolder &) = delete;

    inline ~TextShapeHolder() {
        if (handle)
            textify::destroyTextShapes(TEXTIFY_CONTEXT, &handle, 1);
    }

    inline TextShapeHolder &operator=(textify::TextShapeHandle newHandle) {
        if (handle)
            textify::destroyTextShapes(TEXTIFY_CONTEXT, &handle, 1);
        handle = newHandle;
        return *this;
    }

    inline operator textify::TextShapeHandle() const {
        return handle;
    }

};

constexpr PixelBounds fromTextifyBounds(const textify::Rectangle &bounds) {
    return PixelBounds(
        bounds.l,
        bounds.t,
        bounds.l+bounds.w,
        bounds.t+bounds.h
    );
}

constexpr UnscaledBounds fromTextifyBounds(const textify::FRectangle &bounds) {
    return UnscaledBounds(
        bounds.l,
        bounds.t,
        bounds.l+bounds.w,
        bounds.t+bounds.h
    );
}

constexpr Matrix3x3d fromTextifyMatrix(const textify::Matrix3f &m) {
    return Matrix3x3d(
        m.m[0][0], m.m[0][1], m.m[0][2],
        m.m[1][0], m.m[1][1], m.m[1][2],
        m.m[2][0], m.m[2][1], m.m[2][2]
    );
}

}
