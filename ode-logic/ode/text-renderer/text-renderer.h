
#pragma once

#include <memory>
#include <open-design-text-renderer/text-renderer-api.h>
#include <ode-essentials.h>
#include "../core/bounds.h"
#include "text-renderer-instance.h"
#include "FontBase.h"

namespace ode {

class TextShapeHolder {

    odtr::TextShapeHandle handle;

public:
    class RendererData {
    public:
        virtual ~RendererData() = default;
    };

    std::unique_ptr<RendererData> rendererData;

    inline TextShapeHolder(odtr::TextShapeHandle handle = nullptr) : handle(handle) { }

    TextShapeHolder(const TextShapeHolder &) = delete;

    inline ~TextShapeHolder() {
        if (handle)
            odtr::destroyTextShapes(TEXT_RENDERER_CONTEXT, &handle, 1);
    }

    TextShapeHolder &operator=(TextShapeHolder &&orig) = default;

    inline TextShapeHolder &operator=(odtr::TextShapeHandle newHandle) {
        if (handle)
            odtr::destroyTextShapes(TEXT_RENDERER_CONTEXT, &handle, 1);
        handle = newHandle;
        rendererData.reset();
        return *this;
    }

    inline operator odtr::TextShapeHandle() const {
        return handle;
    }

};

constexpr PixelBounds fromTextRendererBounds(const odtr::Rectangle &bounds) {
    return PixelBounds(
        bounds.l,
        bounds.t,
        bounds.l+bounds.w,
        bounds.t+bounds.h
    );
}

constexpr UnscaledBounds fromTextRendererBounds(const odtr::FRectangle &bounds) {
    return UnscaledBounds(
        bounds.l,
        bounds.t,
        bounds.l+bounds.w,
        bounds.t+bounds.h
    );
}

constexpr Matrix3x3d fromTextRendererMatrix(const odtr::Matrix3f &m) {
    return Matrix3x3d(
        m.m[0][0], m.m[0][1], m.m[0][2],
        m.m[1][0], m.m[1][1], m.m[1][2],
        m.m[2][0], m.m[2][1], m.m[2][2]
    );
}

}
