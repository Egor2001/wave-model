#ifndef WAVE_MODEL_PARALLEL_SOLVER2D_H_
#define WAVE_MODEL_PARALLEL_SOLVER2D_H_

/** 
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include "logging/macro.h"

#include "parallel/abstract_executor.h"
#include "parallel/grid_graph.h"
#include "parallel/counting_semaphore.h"

#include <vector>
#include <algorithm>

#include <cstdint>

/// @brief
namespace wave_model {

/**
 * @brief Solver with parallel execution strategy support
 * @tparam TG Grid type
 *
 * Provides interface over grid, allowing to simply process it.
 * Types and constants are inherited from grid.
 */
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
    static constexpr size_t NMod = TStencil::NMod;

    static_assert(NMod > 0, "NMod must be positive");

/*
    static_assert(NSizeX >= (1u << NRank), 
                  "NSizeX must be equal to 1u << NRank");
*/
    /**
     * @brief Ctor from domain length and time delta
     * @param length Domain side length
     * @param dtime Time discretization delta
     */
    WmParallelSolver2D(double length, double dtime):
        length_(length),
        stencil_(length_ / NSizeY, dtime),
        grid_(layers_arr_, stencil_),
        grid_graph_(grid_.build_graph())
    {}

    /**
     * @brief Ctor from domain length, time delta and initial state function
     * @tparam TInitFunc Initial state function type
     * @param length Domain length
     * @param dtime Time delta
     * @param init_func Initial state function
     */
    template<typename TInitFunc>
    WmParallelSolver2D(double length, double dtime, TInitFunc&& init_func):
        WmParallelSolver2D(length, dtime)
    {
        layers_arr_[NMod - 1]
            .init(length_, std::forward<TInitFunc>(init_func));
    }

    /**
     * @brief Returns current top layer
     * @return Current top layer
     */
    const TLayer& layer() const noexcept
    {
        return layers_arr_[NMod - 1];
    }

    /**
     * @brief Executes proc_cnt calculation steps
     * @param executor Object to execute grid nodes
     * @param proc_cnt Number of steps to do
     */
    void advance(WmAbstractExecutor& executor, size_t proc_cnt)
    {
        static constexpr size_t NShift = (1u << NTileRank) % NMod;

        size_t proc_idx = 0;
        for (; proc_idx < proc_cnt; proc_idx += (1u << NTileRank))
        {
            // make computations in grid order
            for (size_t idx : grid_graph_.order)
            {
                executor.enqueue([this, idx]() {
                        for (size_t affect : grid_graph_.graph[idx])
                            semaphores_[affect].acquire();

                        grid_.access_node(idx)->execute();
                        semaphores_[idx].release();

                        for (size_t affect : grid_graph_.graph[idx])
                            semaphores_[affect].release();
                    });
            }

            // rotate right to emulate dynamic programming with limited memory
            std::rotate(std::rbegin(layers_arr_), 
                        std::rbegin(layers_arr_) + TStencil::NDepth, 
                        std::rend(layers_arr_));
        }
/*
        // rotate left to put result in TStencil::NDepth's position
        size_t extra_cnt = proc_idx - proc_cnt; 
        std::rotate(std::begin(layers_arr_), 
                    std::begin(layers_arr_) + extra_cnt, 
                    std::end(layers_arr_));
*/
    }

private:
    double length_ = 0.0;
    TStencil stencil_;
    TGrid grid_;
    WmGridGraph grid_graph_;
    WmCountingSemaphore<1> semaphores_[TGrid::NNodes];
    TLayer layers_arr_[NMod];
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_SOLVER2D_H_
