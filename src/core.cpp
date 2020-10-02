#include "core.h"

#include <cmath>
#include <cstdio>

void test_core(double factor, double dspace)
{
    using TLayer = WmLayer2D<WmBasicWaveData, 64>;
    using TStencil = WmBasicWaveStencil;
    static constexpr int64_t NSize = static_cast<int64_t>(TLayer::NSize);

    double dtime = dspace / (factor * 2.0);

    auto init_func = [factor, dspace]
        (int64_t x_idx, int64_t y_idx) -> WmBasicWaveData
    {
        double x = static_cast<double>(x_idx - NSize / 2) * dspace;
        double y = static_cast<double>(y_idx - NSize / 2) * dspace;
        double r = sqrt(x * x + y * y);

        return {
            .factor = factor,
            .intencity = sin(r) / (r + 1.0)
        };
    }; 

    TLayer init_layer(init_func);
    TStencil stencil(dspace, dtime);

    WmSolver<TStencil, TLayer::NSize> solver(stencil, std::move(init_layer));
    solver.advance(static_cast<uint64_t>(100.0 / dtime));

    [[maybe_unused]]
    const auto& result_layer = solver.get_layer();

    for (uint64_t x_idx = 0; x_idx < TLayer::NSize; ++x_idx)
    {
        for (uint64_t y_idx = 0; y_idx < TLayer::NSize; ++y_idx)
        {
            int64_t idx = y_idx * TLayer::NSize + x_idx;
            printf("%lf ", result_layer.get(idx).intencity);
        }

        printf("\n");
    }
}
