
#include "TransformationMatrix.h"

#include <cmath>

namespace ode {

const TransformationMatrix TransformationMatrix::identity(1, 0, 0, 1, 0, 0);

TransformationMatrix TransformationMatrix::rotate(double a) {
    return TransformationMatrix(cos(a), sin(a), -sin(a), cos(a), 0, 0);
}

}
