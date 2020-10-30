#ifndef WAVE_MODEL_PARALLEL_SOLVER2D_H_
#define WAVE_MODEL_PARALLEL_SOLVER2D_H_

//TODO: to include in each place where is needed
#include "logging/macro.h"

#include "parallel/abstract_executor.h"

#include <vector>
#include <algorithm>

#include <cstdint>

namespace wave_model {

template<typename TG>
class WmParallelSolver2D
{
public:
    using TGrid = TG;
    using TStencil = typename TGrid::TStencil;
    using TTiling = typename TGrid::TTiling;
    using TLayer = typename TGrid::TLayer;

    static constexpr size_t NCellRank = TGrid::NCellRank;
    static constexpr size_t NRankX = TLayer::NRankX;
    static constexpr size_t NRankY = TLayer::NRankY;

    static constexpr size_t NTileRank = TTiling::NTileRank;
    static constexpr size_t NSizeX = TLayer::NDomainLengthX;
    static constexpr size_t NSizeY = TLayer::NDomainLengthY;
    static constexpr size_t NDepth = TStencil::NDepth + TTiling::NDepth;
/*
    static_assert(NSizeX >= (1u << NRank), 
                  "NSizeX must be equal to 1u << NRank");
*/
    WmParallelSolver2D(double length, double dtime):
        length_(length),
        stencil_(length_ / NSizeY, dtime),
        layers_arr_{},
        grid_(layers_arr_, stencil_)
    {}

    template<typename TInitFunc>
    WmParallelSolver2D(double length, double dtime, TInitFunc&& init_func):
        WmParallelSolver2D(length, dtime)
    {
        layers_arr_[TStencil::NDepth - 1]
            .init(length_, std::forward<TInitFunc>(init_func));
    }

    const TLayer& layer() const noexcept
    {
        return layers_arr_[TStencil::NDepth - 1];
    }

    void advance(WmAbstractExecutor& executor, size_t proc_cnt)
    {
        size_t proc_idx = 0;
        for (; proc_idx < proc_cnt; proc_idx += TTiling::NDepth)
        {
            // make computations for TTiling::NDepth layers
            grid_.traverse(executor);

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
    double length_ = 0.0;
    TStencil stencil_;
    TLayer layers_arr_[NDepth];
    TGrid grid_;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_SOLVER2D_H_