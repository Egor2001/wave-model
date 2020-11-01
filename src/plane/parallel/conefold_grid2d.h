#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include "grid_graph.h"
#include "conefold_node2d.h"
#include "abstract_grid.h"

#include <vector>
#include <algorithm>
#include <cstdint>

// TODO: to move to separate header
#if __has_cpp_attribute(likely) && __has_cpp_attribute(unlikely)
    #define WM_LIKELY [[likely]]
    #define WM_UNLIKELY [[unlikely]]
#else
    #define WM_LIKELY
    #define WM_UNLIKELY
#endif

/// @brief
namespace wave_model {

// TODO: error-checking and tests
/**
 * @brief Describes Grid for ConeFold tiling family
 * @tparam TL Layer type
 * @tparam TS Stencil type
 * @tparam TT Tiling type (must be one of conefold tilings to work correctly)
 * @tparam NR Node rank
 */
template<typename TL, typename TS, typename TT, size_t NR>
class WmConeFoldGrid2D : public WmAbstractGrid
{
public:
    // TODO: struct Test;

    using TLayer = TL;
    using TStencil = TS;

    static constexpr size_t NCellRank = NR;
    static constexpr size_t NCellSide = (1u << NCellRank);

    static constexpr int64_t NCellCountX = 
        (TLayer::NDomainLengthX / NCellSide) + 1;
    static constexpr int64_t NCellCountY = 
        (TLayer::NDomainLengthY / NCellSide) + 1;

    using TNode = WmConeFoldNode2D<TLayer, TStencil, NCellRank>;
    using TTiling = typename TNode::TTiling;
    using EType = typename TTiling::EType;

    static constexpr size_t NTime = TLayer::NDomainLengthX / NCellSide;
    static constexpr size_t NNodes = NCellCountX * NCellCountY * NTime;

    static_assert(NCellSide < TLayer::NDomainLengthX, 
                  "cell must be less than domain");

    /**
     * @brief Ctor from layers array and stencil
     * Initializes internal nodes and additional data
     */
    WmConeFoldGrid2D(TLayer* layers, TStencil& stencil):
        layers_{ layers },
        stencil_{ stencil },
        nodes_{}
    {
        for (size_t cur_time = 0; cur_time < NTime; ++cur_time)
        {
            int64_t row_idx = 0;
            for (int64_t y_idx = 0; y_idx < NCellCountY; ++y_idx)
            {
                int64_t idx = row_idx;
                for (int64_t x_idx = 0; x_idx < NCellCountX; ++x_idx)
                {
                    EType type_x = EType::TYPE_N;
                    switch (x_idx)
                    {
                        case 0: type_x = EType::TYPE_A; break;
                        case 1: type_x = EType::TYPE_B; break;
                        case NCellCountX - 1: 
                                type_x = EType::TYPE_D; break;
                        default: 
                                type_x = EType::TYPE_C; break;
                    }

                    EType type_y = EType::TYPE_N;
                    switch (y_idx)
                    {
                        case 0: type_y = EType::TYPE_A; break;
                        case 1: type_y = EType::TYPE_B; break;
                        case NCellCountY - 1: 
                                type_y = EType::TYPE_D; break;
                        default: 
                                type_y = EType::TYPE_C; break;
                    }

                    int64_t node_idx = NCellCountX * NCellCountY * cur_time + 
                                       y_idx * NCellCountX + x_idx;

                    nodes_[node_idx] = TNode(idx, type_x, type_y, 
                                             &stencil_, layers_);

                    idx += TLayer::template off_right<NCellRank>(idx, 1u);
                }

                row_idx += TLayer::template off_bottom<NCellRank>(row_idx, 1u);
            }
        }
    }

    /**
     * @brief Returns pointer to idx'th node
     * @see WmAbstractGrid::access_node()
     */
    TNode* access_node(size_t idx) override final
    {
        return nodes_ + idx;
    }

    /**
     * @brief Builds and returns conefold structure graph
     * @see WmAbstractGrid::build_graph()
     */
    WmGridGraph build_graph() const override final
    {
        static constexpr int64_t NDiagCnt = NCellCountX + NCellCountY - 1;

        WmGridGraph graph = {};
        graph.count = NTime * NCellCountY * NCellCountX;
        graph.order.reserve(graph.count);
        graph.graph.resize(graph.count);

        for (size_t cur_time = 0; cur_time < NTime; ++cur_time)
        {
            for (int64_t idx = 0; idx < NCellCountY * NCellCountX; ++idx)
            {
                int64_t node_idx = 
                    NCellCountX * NCellCountY * cur_time + idx;

                int64_t x_idx = idx % NCellCountX;
                int64_t y_idx = idx / NCellCountX;

                int64_t add_x = 1;
                int64_t add_y = NCellCountX;
                int64_t add_time = NCellCountY * NCellCountX;

                if (x_idx < NCellCountX - 1)
                {
                    graph.graph[node_idx]
                        .push_back(node_idx + 1);
                }

                if (y_idx < NCellCountY - 1)
                {
                    graph.graph[node_idx]
                        .push_back(node_idx + add_y);
                }

                if (x_idx > 0 && y_idx > 0 && cur_time > 0)
                {
                    graph.graph[node_idx]
                        .push_back(node_idx - add_time - add_x - add_y);
                }
            }

            for (int64_t diag = NDiagCnt - 1; diag >= 0; --diag)
            {
                int64_t max_x_idx = std::min(diag, NCellCountX - 1);
                int64_t max_y_idx = std::min(diag, NCellCountY - 1);

                for (int64_t y_idx = max_y_idx, x_idx = diag - y_idx; 
                     x_idx <= max_x_idx; --y_idx, ++x_idx)
                {
                    graph.order.push_back(
                            cur_time * NCellCountY * NCellCountX + 
                            y_idx * NCellCountY + x_idx);
                }
            }
        }

        return graph;
    }

private:
    TLayer* layers_;
    TStencil& stencil_;
    TNode nodes_[NNodes];
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
