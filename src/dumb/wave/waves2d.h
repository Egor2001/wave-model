#ifndef WAVE_MODEL_WAVE_WAVES2D_H_
#define WAVE_MODEL_WAVE_WAVES2D_H_

#include <cmath>

namespace waves {

struct WmCosineHatWave
{
    double ampl = 1.0;
    double freq = 1.0;

    constexpr double operator () (double x, double y) const noexcept
    {
        double r = std::sqrt(x * x + y * y);

        return ampl * (std::cos(r * freq) + 1.0) / (r + 1.0);
    }
};

struct WmGaussianWave
{
    double ampl = 1.0;
    double sweep2 = 1.0;

    constexpr double operator () (double x, double y) const noexcept
    {
        double r2 = x * x + y * y;

        return ampl * std::exp(-r2 * sweep2);
    }
};

} // namespace waves

#endif // WAVE_MODEL_WAVE_WAVES2D_H_
