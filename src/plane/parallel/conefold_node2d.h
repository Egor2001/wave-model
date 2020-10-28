#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_

#include "abstract_node.h"
#include "counting_semaphore.h"
#include "tiling/general_conefold_tiling2d.h"

namespace wave_model {

template<typename TL, typename TS, size_t NR>
class WmConeFoldNode2D final : public WmAbstractNode
{
public:
    using TLayer = TL;
    using TStencil = TS;

    static constexpr size_t NRank = NR;

    using TTiling = WmGeneralConeFoldTiling2D<NRank>;
    using EType = typename TTiling::EType;

    // TODO: check for layer correctness
    WmConeFoldNode2D(size_t idx, EType type_x, EType type_y, 
                     TStencil& stencil, TLayer* layers):
        WmAbstractNode(),
        idx_{ idx },
        type_x_{ type_x }, 
        type_y_{ type_y },
        stencil_{ stencil },
        layers_{ layers }
    {}

    void execute() override final
    {
        for (auto node : depend_vec_)
            node->semaphore_.acquire();

        proc_fold<EType::TYPE_N, EType::TYPE_N>();
        semaphore_.release(affect_cnt_);
    }

    WmConeFoldNode2D* proceed() const override final
    {
        return next_;
    }

    // pass EType::TYPE_N to determine type
    template<EType NTypeX, EType NTypeY>
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
                default: /* TODO: ERROR! */ break;
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
                default: /* TODO: ERROR! */ break;
            }
        }
        else
        {
            proc_fold<NTypeX, NTypeY>(idx_, stencil_, layers_);
        }
    }

    void next(WmConeFoldNode2D* node)
    {
        next_ = node;
    }

    void depends(WmConeFoldNode2D* node)
    {
        ++node->affect_cnt_;
        depend_vec_.push_back(node);
    }

private:
    WmCountingSemaphore<TStencil::NTargets> semaphore_{ 0 };

    size_t idx_ = 0;
    EType type_x_ = EType::TYPE_N; 
    EType type_y_ = EType::TYPE_N;

    TStencil& stencil_;
    TLayer* layers_ = nullptr;

    size_t affect_cnt_ = 0;
    WmConeFoldNode2D* next_ = nullptr;
    std::vector<WmConeFoldNode2D*> depend_vec_;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
