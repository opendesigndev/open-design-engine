
#pragma once

namespace ode {

/// Texture filtering mode
enum class FilterMode {
    NEAREST,
    LINEAR, // actually a misnomer, maybe fix later
    BILINEAR,
    TRILINEAR
};

}
