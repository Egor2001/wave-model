#ifndef WAVE_MODEL_TEST_LAYER_ZCURVE_LAYER2D_H_
#define WAVE_MODEL_TEST_LAYER_ZCURVE_LAYER2D_H_

#include "layer/zcurve_layer2d.h"

namespace wave_model {

// TODO: to revise structure to avoid copy-paste
template<typename TStream>
TStream& wm_test_zcurve_layer2d(TStream& stream)
{
    struct TestData {};

    constexpr static size_t NDomainLength = 1u << 4;
    constexpr static size_t NCellRank = 2;

    WmZCurveLayer2D<TestData, NDomainLength>::Test::
        template test_off<NCellRank>(stream);
    WmZCurveLayer2D<TestData, NDomainLength>::Test::test_init(stream);
    WmZCurveLayer2D<TestData, NDomainLength>::Test::test_operator(stream);

    return stream;
}

} // namespace wave_model

#endif // WAVE_MODEL_TEST_LAYER_ZCURVE_LAYER2D_H_
