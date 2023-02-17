
#pragma once

#include <array>
#include <string>
#include <vector>
#include <nonstd/optional.hpp>
#include <octopus/general.h>

namespace ode {

/// Represents a single animation of a layer
struct LayerAnimation {

    /// Type of animation
    enum Type {
        TRANSFORM,
        ROTATION,
        OPACITY,
        FILL_COLOR
    };

    /// The state of the animation at a given timepoint
    struct Keyframe {
        /// Difference (in seconds) from the previous keyframe or from zero for the first keyframe
        double delay;
        /// Specifies the parameters of the interpolation equation between the previous and this keyframe - see documentation for more info (not applicable to the first keyframe)
        nonstd::optional<std::vector<double> > easing;

        // Exactly one of the following values depending on type
        /// A 3x2 affine transformation matrix (for TRANSFORM type)
        nonstd::optional<std::array<double, 6> > transform;
        /// Rotation angle (for ROTATION type)
        nonstd::optional<double> rotation;
        /// Opacity multiplier (for OPACITY type)
        nonstd::optional<double> opacity;
        /// Color modifier (for FILL_COLOR type)
        nonstd::optional<octopus::Color> color;
    };

    /// Layer ID specification
    std::string layer;
    /// Animation type
    Type type;
    /// A sequence of animation keyframes
    std::vector<Keyframe> keyframes;
    /// The center of rotation (for ROTATION type only)
    nonstd::optional<std::array<double, 2> > rotationCenter;

};

/// Document animation specifies the set of all layer animation present in a given document (component / design)
struct DocumentAnimation {
    /// The list of all layer animations
    std::vector<LayerAnimation> animations;
};

}
