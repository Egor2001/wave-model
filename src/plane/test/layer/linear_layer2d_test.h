#ifndef WAVE_MODEL_TEST_LAYER_LINEAR_LAYER2D_H_
#define WAVE_MODEL_TEST_LAYER_LINEAR_LAYER2D_H_

#include "layer/linear_layer2d.h"

namespace wave_model {

template<typename TStream>
TStream& wm_test_linear_layer2d(TStream& stream)
{
    struct TestData {};

    constexpr static size_t NDomainLength = 1u << 4;
    constexpr static size_t NCellRank = 2;

    WmLinearLayer2D<TestData, NDomainLength>::Test::
        template test_off<NCellRank>(stream);
    WmLinearLayer2D<TestData, NDomainLength>::Test::test_init(stream);
    WmLinearLayer2D<TestData, NDomainLength>::Test::test_operator(stream);

    return stream;
}

} // namespace wave_model

#endif // WAVE_MODEL_TEST_LAYER_LINEAR_LAYER2D_H_
