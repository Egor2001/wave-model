#define WM_BENCHMARK

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
TStream& test(TStream& stream);

void benchmark();
void test_wave(int argc, char* argv[]);

int main([[maybe_unused]] int argc, 
         [[maybe_unused]] char* argv[])
{
#if defined(WM_BENCHMARK)
    benchmark();
#else
    test_wave(argc, argv);
#endif

    return 0;
}

template<typename TStream>
TStream& test(TStream& stream)
{
    wm_test_linear_layer2d(stream);
    wm_test_zcurve_layer2d(stream);

    wm_test_conefold_tiling2d<WmLinearLayer2D>(stream);
    wm_test_conefold_tiling2d<WmZCurveLayer2D>(stream);

    return stream;
}

void benchmark()
{
    static constexpr size_t NTileRank = 3;
    static constexpr size_t NSideRank = 12;

    static constexpr size_t NRunCount = 
        1ull << ((15 - NSideRank) * 2);

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
               WmLinearLayer2D, NSideRank> 
        solver(1e2, 0.1, init_func);

    solver.advance(NRunCount);
}

void test_wave(int argc, char* argv[])
{
    static constexpr size_t NTileRank = 4;
    static constexpr size_t NSideRank = 7;
    static constexpr size_t NRunCount = 100; 

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

    if (argc < 2)
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        test(WmLogger::stream());
    }

    solver.advance(NRunCount);
    solver.layer().dump(out_stream);
}
