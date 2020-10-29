#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_

#include "conefold_node2d.h"
#include "abstract_executor.h"

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

namespace wave_model {

// TODO: error-checking and tests
template<typename TL, typename TS, typename TT, size_t NR>
class WmConeFoldGrid2D
{
public:
    struct Test;

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

    static_assert(NCellSide < TLayer::NDomainLengthX, 
                  "cell must be less than domain");

    WmConeFoldGrid2D(TLayer* layers, TStencil& stencil):
        layers_{ layers },
        stencil_{ stencil }
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

                    nodes_[node_idx].init(idx, type_x, type_y, 
                                          &stencil_, layers_);

                    int64_t add_x = 1;
                    int64_t add_y = NCellCountX;

                    if (x_idx < NCellCountX - 1)
                        nodes_[node_idx].depends(nodes_ + (node_idx + add_x));

                    if (y_idx < NCellCountY - 1)
                        nodes_[node_idx].depends(nodes_ + (node_idx + add_y));

                    if (x_idx < NCellCountX - 1 && y_idx < NCellCountY - 1)
                    {
                        nodes_[node_idx].next(nodes_ + 
                                (node_idx + add_x + add_y));
                    }

                    idx += TLayer::template off_right<NCellRank>(idx, 1u);
                }

                row_idx += TLayer::template off_bottom<NCellRank>(row_idx, 1u);
            }
        }
    }

    void traverse(WmAbstractExecutor& executor)
    {
        static constexpr int64_t NDiagCnt = NCellCountX + NCellCountY - 1;
        for (int64_t diag = NDiagCnt - 1; diag >= 0; --diag)
        {
            int64_t max_x_idx = std::min(diag, NCellCountX - 1);
            int64_t max_y_idx = std::min(diag, NCellCountY - 1);

            for (int64_t y_idx = max_y_idx, x_idx = diag - y_idx; 
                 x_idx <= max_x_idx; --y_idx, ++x_idx)
            {
                int64_t idx = y_idx * NCellCountY + x_idx;
                launch(executor, nodes_ + idx);
            }
        }

        for (size_t cur_time = 1; cur_time < NTime; ++cur_time)
        {
            for (int64_t diag = std::max(NCellCountX, NCellCountY) - 1; 
                 diag >= 0; --diag)
            {
                WM_UNLIKELY if (diag == 0)
                {
                    launch(executor, nodes_ + (NCellCountX * NCellCountY * 
                                               cur_time));

                    continue;
                }

                WM_LIKELY if (diag < NCellCountX)
                {
                    launch(executor, nodes_ + (NCellCountX * NCellCountY * 
                                               cur_time + diag));
                }

                WM_LIKELY if (diag < NCellCountY)
                {
                    launch(executor, nodes_ + (NCellCountX * NCellCountY * 
                                               cur_time + NCellCountX * diag));
                }
            }
        }
    }

    void launch(WmAbstractExecutor& executor, TNode* node)
    {
        auto proc = [node]() mutable
        {
            while (node != nullptr)
            {
                node->execute();
                node = node->proceed();
            }
        };

        executor.enqueue(proc);
    }

private:
    TLayer* layers_;
    TStencil& stencil_;
    TNode nodes_[NCellCountX * NCellCountY * NTime];
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
