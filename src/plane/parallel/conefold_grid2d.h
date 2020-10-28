#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_

#include "conefold_node2d.h"
#include "abstract_executor.h"

#include <algorithm>
#include <cstdint>

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
    static constexpr size_t NCellCount = 
        ((TLayer::NDomainSizeX + NCellSide) * 
         (TLayer::NDomainSizeY + NCellSide)) / (NCellSide * NCellSide);

    using TNode = WmConeFoldNode2D<TLayer, TStencil, NCellRank>;
    using TTiling = typename TNode::TTiling;
    using EType = typename TTiling::EType;

    static constexpr size_t NTime = TLayer::NDomainSizeX / NCellSide;

    static_assert(NCellSide < TLayer::NDomainSizeX, 
                  "cell must be less than domain");

    explicit WmConeFoldGrid2D(TLayer* layers, TStencil& stencil):
        layers_{ layers },
        stencil_{ stencil }
    {
        int64_t row_idx = 0;
        for (int64_t y_idx = 0; y_idx <= TLayer::NDomainSizeY; 
             y_idx += NCellSide)
        {
            int64_t idx = row_idx;
            for (int64_t x_idx = 0; x_idx <= TLayer::NDomainSizeX; 
                 x_idx += NCellSide)
            {
                int64_t node_idx = 
                    (TLayer::NDomainSizeX / NCellSide) * y_idx + x_idx;

                EType type_x = EType::TYPE_N;
                switch (x_idx)
                {
                    case 0:                    type_x = EType::TYPE_A; break;
                    case 1:                    type_x = EType::TYPE_B; break;
                    case TLayer::NDomainSizeX: type_x = EType::TYPE_D; break;
                    default:                   type_x = EType::TYPE_C; break;
                }

                EType type_y = EType::TYPE_N;
                switch (y_idx)
                {
                    case 0:                    type_y = EType::TYPE_A; break;
                    case 1:                    type_y = EType::TYPE_B; break;
                    case TLayer::NDomainSizeY: type_y = EType::TYPE_D; break;
                    default:                   type_y = EType::TYPE_C; break;
                }

                nodes_[node_idx] = 
                    TNode(idx, type_x, type_y, layers_, stencil_);

                int64_t add_x = TLayer::template 
                    off_right<NCellRank>(node_idx, 1u);
                int64_t add_y = TLayer::template 
                    off_bottom<NCellRank>(node_idx, 1u);

                if (x_idx < TLayer::NDomainSizeX)
                    nodes_[node_idx].depends(nodes_ + (node_idx + add_x));

                if (y_idx < TLayer::NDomainSizeY)
                    nodes_[node_idx].depends(nodes_ + (node_idx + add_y));

                if (x_idx < TLayer::NDomainSizeX && 
                    y_idx < TLayer::NDomainSizeY)
                    nodes_[node_idx].next(nodes_ + (node_idx + add_x + add_y));

                idx += TLayer::template 
                    off_right<NCellRank>(idx, 1u);
            }

            row_idx += TLayer::template 
                off_bottom<NCellRank>(row_idx, 1u);
        }
    }

    void traverse(WmAbstractExecutor& executor)
    {
        int64_t idx = 0;
        for (idx = TLayer::NDomainSizeX * TLayer::NDomainSizeY - 
             NCellSide * NCellSide; idx >= 0; idx -= NCellSide * NCellSide)
        {
            idx += (TTiling::template off_right<NCellRank>(idx, 1u) + 
                    TTiling::template off_bottom<NCellRank>(idx, 1u));
            launch(executor, nodes_ + idx);
        }

        std::vector<int64_t> border_vec;
        border_vec.push_back(0);

        idx = 0;
        for (int64_t col = 1; col <= TLayer::NDomainSizeX; col += NCellSide)
        {
            idx += TTiling::template off_right<NCellRank>(idx, 1);
            border_vec.push_back(idx);
        }

        idx = 0;
        for (int64_t row = 1; row <= TLayer::NDomainSizeY; row += NCellSide)
        {
            idx += TTiling::template off_bottom<NCellRank>(idx, 1);
            border_vec.push_back(idx);
        }

        std::sort(std::begin(border_vec), std::end(border_vec), 
                  std::greater<int64_t>{});

        for (size_t cur_time = 1; cur_time < NTime; ++cur_time)
        {
            for (int64_t border_idx : border_vec)
                launch(executor, nodes_ + (NCellCount * cur_time + border_idx));
        }
    }

    void launch(WmAbstractExecutor& executor, TNode* node)
    {
        auto proc = [this, node]()
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
    TNode nodes_[NCellCount * NTime];
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_GRID_2D_H_
