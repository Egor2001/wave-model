#include "solver2d.h"

#include "test/tiling/conefold_tiling2d_test.h"
#include "test/layer/linear_layer2d_test.h"
#include "test/layer/zcurve_layer2d_test.h"

#include <iostream>

using namespace wave_model;

void test()
{
    wm_test_linear_layer2d(std::cout);
    wm_test_zcurve_layer2d(std::cout);

    wm_test_conefold_tiling2d<WmLinearLayer2D>(std::cout);
    wm_test_conefold_tiling2d<WmZCurveLayer2D>(std::cout);
}

int main()
{
    // test();

    WmCosineHatWave2D init_wave { .ampl = 1.0, .freq = 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            .factor = 1.0,
            .intencity = init_wave(x, y) 
        }; 
    };

    static constexpr size_t NTileRank = 2;
    static constexpr size_t NSideRank = 10;

    static constexpr size_t NRunCount = // 1; // for cachegrind benchmark
        1ull << ((16 - NSideRank) * 2 - NTileRank);

    WmSolver2D<WmBasicWaveStencil2D, 
               WmConeFoldTiling2D<NTileRank>, 
               WmLinearLayer2D, NSideRank> 
        solver(1e6, 0.1, init_func);

    solver.advance(NRunCount);

    return 0;
}
