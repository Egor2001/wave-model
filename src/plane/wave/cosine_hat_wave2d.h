#ifndef WAVE_MODEL_WAVE_COSINE_HAT_WAVE2D_H
#define WAVE_MODEL_WAVE_COSINE_HAT_WAVE2D_H

#include <cmath>

namespace wave_model {

struct WmCosineHatWave2D
{
    double ampl = 1.0;
    double freq = 1.0;

    constexpr double operator () (double x, double y) const noexcept
    {
        double r = std::sqrt(x * x + y * y);

        return ampl * (std::cos(r * freq) + 1.0) / (r + 1.0);
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_WAVE_COSINE_HAT_WAVE2D_H_
