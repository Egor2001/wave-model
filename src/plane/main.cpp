// #define WM_BENCHMARK

#include "solver2d.h"
#include "logging/macro.h"
#include "logging/logger.h"

#include "test/tiling/conefold_tiling2d_test.h"
#include "test/layer/linear_layer2d_test.h"
#include "test/layer/zcurve_layer2d_test.h"

#include <iostream>
#include <fstream>

using namespace wave_model;

template<typename TStream>
TStream& test(TStream& stream)
{
    wm_test_linear_layer2d(stream);
    wm_test_zcurve_layer2d(stream);

    wm_test_conefold_tiling2d<WmLinearLayer2D>(stream);
    wm_test_conefold_tiling2d<WmZCurveLayer2D>(stream);

    return stream;
}

int main([[maybe_unused]] int argc, 
         [[maybe_unused]] char* argv[])
{
#if defined(WM_BENCHMARK)
    static constexpr size_t NTileRank = 2;
    static constexpr size_t NSideRank = 10;

    static constexpr size_t NRunCount = // 1; // for cachegrind benchmark
        1ull << ((16 - NSideRank) * 2 - NTileRank);

#else // if !defined(WM_BENCHMARK)
    static constexpr size_t NTileRank = 4;
    static constexpr size_t NSideRank = 7;
    static constexpr size_t NRunCount = 100; 

#endif // defined(WM_BENCHMARK)

    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            /* .factor = */ 1.0,
            /* .intencity = */ init_wave(x, y) 
        }; 
    };

    WmSolver2D<WmBasicWaveStencil2D, 
               WmConeFoldTiling2D<NTileRank>, 
               WmZCurveLayer2D, NSideRank> 
        solver(1e2, 0.1, init_func);

#if defined(WM_BENCHMARK)
    solver.advance(NRunCount);

#else // if !defined(WM_BENCHMARK)
    if (argc < 2)
    {
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";
        return 1;
    }

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        test(WmLogger::stream());
    }

    auto& layer = solver.advance(NRunCount);
    layer.dump(out_stream);

#endif // defined(WM_BENCHMARK)

    return 0;
}
