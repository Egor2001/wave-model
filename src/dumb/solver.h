#ifndef WAVE_MODEL_SOLVER_SPACE_H_
#define WAVE_MODEL_SOLVER_SPACE_H_

#include <vector>
#include <algorithm>

#include <cstdint>

template<size_t NS, size_t ND>
class WmSolver2D
{
public:
    using TStencil = WmBasicWaveStencil2D;
    using TTiling = WmConeFoldTiling2D;
    using TLayer = WmZCurveLayer2D<NS>;

    static constexpr size_t NSize = NS;
    static constexpr size_t NDepth = TStencil::NDepth + TTiling::NDepth;

    WmSolver2D(double length, const TStencil& stencil):
        length_(length),
        stencil_(stencil),
        layers_arr_{}
    {}

    template<typename TInitFunc>
    WmSolver2D(double length, const TStencil& stencil, TInitFunc&& init_func):
        WmSolver2D(length, stencil)
    {
        layers_.front().init(length_, std::forward<TInitFunc>(init_func));
    }

    void proceed()
    {
        TTiling::traverse<TLayer::NSize, NRank>([](uint64_t idx) {
            TTiling::proc_fold<NRank, TStencil, TLayer>
                (idx, stencil, layers_arr_ + TStencil::NDepth);
        });

        for (size_t y_idx = 0; y_idx < TLayer::NSize; y_idx += NQuadSize)
        for (size_t x_idx = 0; x_idx < TLayer::NSize; x_idx += NQuadSize)
        {
            TTiling::proc_fold<NRank, TStencil, TLayer>
                (x_idx, y_idx, stencil, layers_arr_ + TStencil::NDepth);
        }
    }

    void advance(size_t proc_cnt)
    {
        size_t proc_idx = 0;
        for (; proc_idx < proc_cnt; proc_idx += TTiling::NDepth)
        {
            // make computations for TTiling::NDepth layers
            proceed();

            // rotate right to emulate dynamic programming with limited memory
            std::rotate(std::rbegin(layers_), 
                        std::rbegin(layers_) + TStencil::NDepth, 
                        std::rend(layers));
        }

        // rotate left to put result in TStencil::NDepth's position
        size_t extra_cnt = proc_idx - proc_cnt; 
        std::rotate(std::begin(layers_), 
                    std::begin(layers_) + extra_cnt, 
                    std::end(layers));
    }

private:
    double length_;
    TStencil stencil_;
    TLayer layers_arr_[NDepth];
};

#endif // WAVE_MODEL_SOLVER_SPACE_H_
