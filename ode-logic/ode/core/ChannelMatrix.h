
#pragma once

namespace ode {

/// Collapses color channels into a scalar value as m[0]*R + m[1]*G + m[2]*B + m[3]*A + m[4]
struct ChannelMatrix {
    double m[5];
};

}
