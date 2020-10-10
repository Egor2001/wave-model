#include "solver2d.h"

#include "test/tiling/conefold_tiling2d_test.h"
#include "test/layer/linear_layer2d_test.h"
#include "test/layer/zcurve_layer2d_test.h"

#include <iostream>

using namespace wave_model;

int main()
{
    wm_test_linear_layer2d(std::cout);
    wm_test_zcurve_layer2d(std::cout);

    wm_test_conefold_tiling2d<WmLinearLayer2D>(std::cout);
    wm_test_conefold_tiling2d<WmZCurveLayer2D>(std::cout);

    WmCosineHatWave2D init_wave { .ampl = 1.0, .freq = 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            .factor = 1.0,
            .intencity = init_wave(x, y) 
        }; 
    };

    WmSolver2D<WmBasicWaveStencil2D, 
               WmConeFoldTiling2D<2>, 
               WmLinearLayer2D, 4> 
        solver(10.0, 0.1, init_func);

    solver.advance(1);

    return 0;
}
