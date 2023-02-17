
#include "animate.h"

#include <cmath>
#include <cstring>

namespace ode {

struct AnimationProgress {
    const LayerAnimation::Keyframe *a, *b;
    double t;
};

double animationKeyframeProgress(const LayerAnimation::Keyframe &keyframe, double time) {
    if (time < 0)
        return 0;
    if (time >= keyframe.delay)
        return 1;
    double t = time/keyframe.delay;
    if (keyframe.easing.has_value()) {
        std::vector<double> p(keyframe.easing->size()+2);
        p.front() = 0;
        memcpy(&p[1], keyframe.easing->data(), sizeof(double)*keyframe.easing->size());
        p.back() = 1;
        while (p.size() > 1) {
            for (size_t i = 0; i < p.size()-1; ++i)
                p[i] = (1-t)*p[i]+t*p[i+1];
            p.pop_back();
        }
        return p.front();
    }
    return t;
}

AnimationProgress animationProgress(const LayerAnimation &animation, double time) {
    AnimationProgress progress = { };
    if (!animation.keyframes.empty()) {
        progress.a = progress.b = &animation.keyframes[0];
        for (const LayerAnimation::Keyframe &keyframe : animation.keyframes) {
            if (time < keyframe.delay) {
                progress.b = &keyframe;
                progress.t = animationKeyframeProgress(keyframe, time);
                return progress;
            }
            time -= keyframe.delay;
            progress.a = &keyframe;
        }
        progress.b = progress.a;
    }
    return progress;
}

TransformationMatrix animateTransform(const LayerAnimation &animation, double time) {
    switch (animation.type) {
        case LayerAnimation::TRANSFORM:
            {
                AnimationProgress progress = animationProgress(animation, time);
                if (progress.a && progress.b) {
                    ODE_ASSERT(progress.a->transform.has_value() && progress.b->transform.has_value());
                    if (progress.a->transform.has_value() && progress.b->transform.has_value()) {
                        return TransformationMatrix(
                            (1-progress.t)*progress.a->transform.value()[0]+progress.t*progress.b->transform.value()[0],
                            (1-progress.t)*progress.a->transform.value()[1]+progress.t*progress.b->transform.value()[1],
                            (1-progress.t)*progress.a->transform.value()[2]+progress.t*progress.b->transform.value()[2],
                            (1-progress.t)*progress.a->transform.value()[3]+progress.t*progress.b->transform.value()[3],
                            (1-progress.t)*progress.a->transform.value()[4]+progress.t*progress.b->transform.value()[4],
                            (1-progress.t)*progress.a->transform.value()[5]+progress.t*progress.b->transform.value()[5]
                        );
                    }
                }
            }
            break;
        case LayerAnimation::ROTATION:
            ODE_ASSERT(animation.rotationCenter.has_value());
            if (animation.rotationCenter.has_value()) {
                double rotation = animateRotation(animation, time);
                return (
                    TransformationMatrix(1, 0, 0, 1, animation.rotationCenter.value()[0], animation.rotationCenter.value()[1])*
                    TransformationMatrix(cos(rotation), sin(rotation), -sin(rotation), cos(rotation), 0, 0)*
                    TransformationMatrix(1, 0, 0, 1, -animation.rotationCenter.value()[0], -animation.rotationCenter.value()[1])
                );
            }
            break;
        default:;
    }
    return TransformationMatrix::identity;
}

double animateRotation(const LayerAnimation &animation, double time) {
    if (animation.type == LayerAnimation::ROTATION) {
        AnimationProgress progress = animationProgress(animation, time);
        if (progress.a && progress.b) {
            ODE_ASSERT(progress.a->rotation.has_value() && progress.b->rotation.has_value());
            if (progress.a->rotation.has_value() && progress.b->rotation.has_value())
                return (1-progress.t)*progress.a->rotation.value()+progress.t*progress.b->rotation.value();
        }
    }
    return 0;
}

double animateOpacity(const LayerAnimation &animation, double time) {
    if (animation.type == LayerAnimation::OPACITY) {
        AnimationProgress progress = animationProgress(animation, time);
        if (progress.a && progress.b) {
            ODE_ASSERT(progress.a->opacity.has_value() && progress.b->opacity.has_value());
            if (progress.a->opacity.has_value() && progress.b->opacity.has_value())
                return (1-progress.t)*progress.a->opacity.value()+progress.t*progress.b->opacity.value();
        }
    }
    return 1;
}

Color animateColor(const LayerAnimation &animation, double time) {
    if (animation.type == LayerAnimation::FILL_COLOR) {
        AnimationProgress progress = animationProgress(animation, time);
        if (progress.a && progress.b) {
            ODE_ASSERT(progress.a->color.has_value() && progress.b->color.has_value());
            if (progress.a->color.has_value() && progress.b->color.has_value()) {
                return Color(
                    (1-progress.t)*progress.a->color.value().r+progress.t*progress.b->color.value().r,
                    (1-progress.t)*progress.a->color.value().g+progress.t*progress.b->color.value().g,
                    (1-progress.t)*progress.a->color.value().b+progress.t*progress.b->color.value().b,
                    (1-progress.t)*progress.a->color.value().a+progress.t*progress.b->color.value().a
                );
            }
        }
    }
    return Color();
}

static std::array<double, 6> convertTransformation(const TransformationMatrix &m) {
    return std::array<double, 6> {
        m[0][0], m[0][1],
        m[1][0], m[1][1],
        m[2][0], m[2][1],
    };
}

static octopus::Color convertColor(const Color &color) {
    octopus::Color result;
    result.r = color.r;
    result.g = color.g;
    result.b = color.b;
    result.a = color.a;
    return result;
}

LayerAnimation::Keyframe animate(const LayerAnimation &animation, double time) {
    LayerAnimation::Keyframe result;
    result.delay = time;
    switch (animation.type) {
        case LayerAnimation::TRANSFORM:
            result.transform = convertTransformation(animateTransform(animation, time));
            break;
        case LayerAnimation::ROTATION:
            result.rotation = animateRotation(animation, time);
            break;
        case LayerAnimation::OPACITY:
            result.opacity = animateOpacity(animation, time);
            break;
        case LayerAnimation::FILL_COLOR:
            result.color = convertColor(animateColor(animation, time));
            break;
    }
    return result;
}

}
