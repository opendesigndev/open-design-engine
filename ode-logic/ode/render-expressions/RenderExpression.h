
#pragma once

#include <memory>
#include <ode-essentials.h>
#include "../core/bounds.h"
#include <octopus/layer.h>

namespace ode {

struct RenderExpression;

typedef RenderExpression Rendexpr;
typedef std::shared_ptr<Rendexpr> Rendexptr;

/// An abstract node of the render expression tree
struct RenderExpression {
    typedef int Type;

    Type type;
    int refs = 0;
    int flags = 0;

    constexpr explicit RenderExpression(Type type, int flags) : type(type), flags(flags) { }
    virtual ~RenderExpression() = default;

    inline virtual const octopus::Layer *getLayer() const {
        return nullptr;
    }

};

}
