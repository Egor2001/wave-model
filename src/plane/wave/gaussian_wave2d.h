#ifndef WAVE_MODEL_WAVE_GAUSSIAN_WAVE2D_H_
#define WAVE_MODEL_WAVE_GAUSSIAN_WAVE2D_H_

#include "logging/macro.h"

#include <cmath>

namespace wave_model {

struct WmGaussianWave2D
{
    double ampl = 1.0;
    double sweep2 = 1.0;

    constexpr double operator () (double x, double y) const noexcept
    {
        double r2 = x * x + y * y;

        return ampl * std::exp(-r2 * sweep2);
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_WAVE_GAUSSIAN_WAVE2D_H_
