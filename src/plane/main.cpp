// #define WM_BENCHMARK

#include "solver2d.h"
#include "general_solver2d.h"
#include "logging/macro.h"
#include "logging/logger.h"

#include "layer/avx_layer2d.h"
#include "layer/general_linear_layer2d.h"
#include "stencil/avx_basic_wave_stencil2d.h"
#include "tiling/general_conefold_tiling2d.h"

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
    static constexpr size_t NTileRank = 3;
    static constexpr size_t NSideRank = 5;
    static constexpr size_t NRunCount = 100; 

    static constexpr double length = 1e2;
    static constexpr double delta = length / (1u << NSideRank);
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmAvxBasicWaveData2D
    { 
        return { 
            // .factor = 
                _mm256_set1_pd(1.0),
            // .intencity = 
                _mm256_setr_pd(init_wave(4.0 * x + 0.0, y), 
                               init_wave(4.0 * x + 1.0, y), 
                               init_wave(4.0 * x + 2.0, y), 
                               init_wave(4.0 * x + 3.0, y)) 
        }; 
    };

    WmGeneralSolver2D<WmAvxBasicWaveStencil2D, 
               WmGeneralConeFoldTiling2D<NTileRank>, 
               WmAvxLinearLayer2D, NSideRank> 
        solver(length, 0.1, init_func);

    if (argc < 2)
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        test(WmLogger::stream());
    }

    // solver.advance(NRunCount);
    solver.layer().dump(out_stream);
}
