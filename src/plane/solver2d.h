#ifndef WAVE_MODEL_SOLVER2D_H_
#define WAVE_MODEL_SOLVER2D_H_

//TODO: to include in each place where is needed
#include "logging/macro.h"
#include "layer/linear_layer2d.h"
#include "layer/zcurve_layer2d.h"
#include "wave/cosine_hat_wave2d.h"
#include "wave/gaussian_wave2d.h"
#include "tiling/conefold_tiling2d.h"
#include "stencil/basic_wave_stencil2d.h"

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

template<typename TS, typename TT, 
         template<typename, size_t> typename TL, size_t NR>
class WmSolver2D
{
public:
    static constexpr size_t NRank = NR;

    using TStencil = TS;
    using TTiling = TT;
    using TLayer = TL<typename TStencil::TData, 1ull << NRank>;

    static constexpr size_t NTileRank = TTiling::NTileRank;
    static constexpr size_t NSize = TLayer::NDomainLength;
    static constexpr size_t NDepth = TStencil::NDepth + TTiling::NDepth;

    WmSolver2D(double length, double dtime):
        length_(length),
        stencil_(length_ / NSize, dtime),
        layers_arr_{}
    {}

    template<typename TInitFunc>
    WmSolver2D(double length, double dtime, TInitFunc&& init_func):
        WmSolver2D(length, dtime)
    {
        layers_arr_[TStencil::NDepth - 1]
            .init(length_, std::forward<TInitFunc>(init_func));
    }

    const TLayer& layer() const noexcept
    {
        return layers_arr_[TStencil::NDepth - 1];
    }

    void advance(size_t proc_cnt)
    {
        size_t proc_idx = 0;
        for (; proc_idx < proc_cnt; proc_idx += TTiling::NDepth)
        {
            // make computations for TTiling::NDepth layers
            TTiling::template traverse<NRank>
                (stencil_, layers_arr_ + TStencil::NDepth);

            // rotate right to emulate dynamic programming with limited memory
            std::rotate(std::rbegin(layers_arr_), 
                        std::rbegin(layers_arr_) + TStencil::NDepth, 
                        std::rend(layers_arr_));
        }

        // rotate left to put result in TStencil::NDepth's position
        size_t extra_cnt = proc_idx - proc_cnt; 
        std::rotate(std::begin(layers_arr_), 
                    std::begin(layers_arr_) + extra_cnt, 
                    std::end(layers_arr_));
    }

private:
    double length_;
    TStencil stencil_;
    TLayer layers_arr_[NDepth];
};

} // namespace wave_model

#endif // WAVE_MODEL_SOLVER2D_H_
