#ifndef WAVE_MODEL_SOLVER_TILING_H_
#define WAVE_MODEL_SOLVER_TILING_H_

#include "layer/linear_layer.h"
#include "layer/zcurve_layer.h"

#include <vector>
#include <algorithm>

#include <cstdint>

struct WmConeFoldTiling2D
{
    template<size_t NRank, typename TStencil, typename TLayer>
    static void traverse(const TStencil& stencil, TLayer* layers) noexcept
    {
        int64_t x_off = TLayer::off_right<NRank>(0, 1);
        int64_t y_off = TLayer::off_bottom<NRank>(0, 1);

        // TODO: to distinguish between the different fold types
        proc_fold<NRank>(x_off + y_off, stencil, layers);
        proc_fold<NRank>(y_off,         stencil, layers);
        proc_fold<NRank>(x_off,         stencil, layers);
        proc_fold<NRank>(0,             stencil, layers);
    }

    // TODO: to generate different fold type handlers with python!
    template<size_t NRank, typename TStencil, typename TLayer>
    static void proc_fold(int64_t idx, 
            const TStencil& stencil, TLayer* layers) noexcept
    {
        WM_ASSERT(idx >= 0, "index must be non-negative");
        // static_assert(NRank > 0, "NRank must be > 0");

        if constexpr (NRank == 0u)
        {
            layers[0][idx] = stencil.apply(idx, layers);
        }
        else
        {
            // TODO: to optimize for z-order case
            int64_t x_off = TLayer::off_right<NRank - 1>(idx, 1);
            int64_t y_off = TLayer::off_bottom<NRank - 1>(idx, 1);

            proc_fold<NRank - 1>(idx + x_off + y_off, stencil, layers);
            proc_fold<NRank - 1>(idx + x_off,         stencil, layers);
            proc_fold<NRank - 1>(idx + y_off,         stencil, layers);
            proc_fold<NRank - 1>(idx,                 stencil, layers);

            ++layers;
            idx += x_off + y_off;
            x_off = TLayer::off_right<NRank - 1>(idx, 1);
            y_off = TLayer::off_bottom<NRank - 1>(idx, 1);

            proc_fold<NRank - 1>(idx + x_off + y_off, stencil, layers);
            proc_fold<NRank - 1>(idx + x_off,         stencil, layers);
            proc_fold<NRank - 1>(idx + y_off,         stencil, layers);
            proc_fold<NRank - 1>(idx,                 stencil, layers);
        }
    }
};

#endif // WAVE_MODEL_SOLVER_TILING_H_
