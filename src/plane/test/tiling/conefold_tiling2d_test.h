#ifndef WAVE_MODEL_TEST_TILING_CONEFOLD_TILING2D_H_
#define WAVE_MODEL_TEST_TILING_CONEFOLD_TILING2D_H_

#include "tiling/conefold_tiling2d.h"
#include "layer/zcurve_layer2d.h"
#include "layer/linear_layer2d.h"

namespace wave_model {

template<typename TStream>
TStream& wm_test_conefold_tiling2d(TStream& stream)
{
    static constexpr size_t NTileRank = 2;
    static constexpr size_t NRank = 4;
    /*
    WmConeFoldTiling2D<NTileRank>::Test::
        test_traverse<NRank, WmZCurveLayer2D, TStream>(stream);
        */

    WmConeFoldTiling2D<NTileRank>::Test::
        test_traverse<NRank, WmLinearLayer2D, TStream>(stream);

    return stream;
}

} // namespace wave_model

#endif // WAVE_MODEL_TEST_TILING_CONEFOLD_TILING2D_H_
