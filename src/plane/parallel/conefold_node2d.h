#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include "abstract_node.h"
#include "counting_semaphore.h"
#include "tiling/general_conefold_tiling2d.h"

/// @brief
namespace wave_model {

/**
 * @brief Describes node class for conefold tiling family
 * @tparam TL Layer type (needs to be one of the conefold family)
 * @tparam TS Stencil type
 * @tparam NR Node rank
 */
template<typename TL, typename TS, size_t NR>
class WmConeFoldNode2D final : public WmAbstractNode
{
public:
    using TLayer = TL;
    using TStencil = TS;

    static constexpr size_t NRank = NR;

    using TTiling = WmGeneralConeFoldTiling2D<NRank>;
    using EType = typename TTiling::EType;

    /**
     * @brief Default ctor
     * Produces invalid node.
     * Only presents to store nodes in an array.
     */
    WmConeFoldNode2D() = default;

    /**
     * @brief Ctor initializing all node fields
     * Needs to be called on each node to make it valid.
     */
    WmConeFoldNode2D(size_t idx, EType type_x, EType type_y, 
                     TStencil* stencil, TLayer* layers):
        idx_{ idx },
        type_x_{ type_x },
        type_y_{ type_y },
        stencil_{ stencil },
        layers_{ layers }
    {}

    /**
     * @brief Executes node calculations
     * @see WmAbstractNode::execute()
     */
    void execute() override final
    {
        proc_fold();
    }

protected:
    template<EType NTypeX = EType::TYPE_N, EType NTypeY = EType::TYPE_N>
    void proc_fold()
    {
        static constexpr EType NTypeN = EType::TYPE_N;

        if constexpr (NTypeX == NTypeN)
        {
            switch (type_x_) 
            {
                case EType::TYPE_A: proc_fold<EType::TYPE_A, NTypeN>(); break;
                case EType::TYPE_B: proc_fold<EType::TYPE_B, NTypeN>(); break;
                case EType::TYPE_C: proc_fold<EType::TYPE_C, NTypeN>(); break;
                case EType::TYPE_D: proc_fold<EType::TYPE_D, NTypeN>(); break;
                case EType::TYPE_N: /* TODO: ERROR! */ break;
                // default: /* TODO: ERROR! */ break;
            }
        }
        else if constexpr (NTypeY == NTypeN)
        {
            switch (type_y_) 
            {
                case EType::TYPE_A: proc_fold<NTypeX, EType::TYPE_A>(); break;
                case EType::TYPE_B: proc_fold<NTypeX, EType::TYPE_B>(); break;
                case EType::TYPE_C: proc_fold<NTypeX, EType::TYPE_C>(); break;
                case EType::TYPE_D: proc_fold<NTypeX, EType::TYPE_D>(); break;
                case EType::TYPE_N: /* TODO: ERROR! */ break;
                // default: /* TODO: ERROR! */ break;
            }
        }
        else
        {
            TTiling::template 
                proc_fold<NRank, NTypeX, NTypeY>(idx_, *stencil_, 
                                                 layers_ + TStencil::NDepth);
        }
    }

private:
    size_t idx_ = 0;
    EType type_x_ = EType::TYPE_N; 
    EType type_y_ = EType::TYPE_N;

    TStencil* stencil_ = nullptr;
    TLayer* layers_ = nullptr;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
