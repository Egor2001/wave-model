#ifndef WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
#define WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_

#include "abstract_node.h"

namespace wave_model {

template<typename TL, typename TT, size_t NR>
class WmConeFoldNode2D final : public WmAbstractNode
{
public:
    using TLayer = TL;
    using TTiling = TT;

    static constexpr size_t NRank = NR;

    // TODO: check for layer correctness
    WmConeFoldNode2D(size_t idx, TStencil& stencil, TLayer* layers):
        WmAbstractNode(),
        idx_{ idx },
        stencil_{ stencil },
        layers_{ layers }
    {}

    void execute() override final
    {
        for (auto node : depend_vec_)
            node->semaphore_.acquire();

        TTiling::proc_fold<NRank>(idx_, stencil_, layers_);
        semaphore_.release(affect_cnt_);
    }

    WmConeFoldNode2D* proceed() const override final
    {
        return next_;
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
    std::counting_semaphore semaphore_{ 0 };

    size_t idx_;
    TStencil& stencil_;
    TLayer* layers_ = nullptr;

    size_t affect_cnt_ = 0;
    WmConeFoldNode2D* next_ = nullptr;
    std::vector<WmConeFoldNode2D*> depend_vec_;
};

} // namespace wave_model

#endif // WAVE_MODEL_PARALLEL_CONEFOLD_NODE_2D_H_
