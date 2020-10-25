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

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_scalar(double length /* = 1e2 */, 
                double delta_time /* = 0.1 */, 
                size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(NTileRank < NSideRank, "tile must be less than side");

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
        solver(length, delta_time, init_func);

    solver.advance(run_count);

    return solver;
}

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_vector(double length /* = 1e2 */, 
                double delta_time /* =  0/1 */, 
                size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(NTileRank < NSideRank, "tile must be less than side");

    double delta = length / (1u << (NSideRank - 2));
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave, delta](double x, double y) -> 
        WmAvxBasicWaveData2D
    { 
        return { 
            // .factor = 
                _mm256_set1_pd(1.0),
            // .intencity = 
            // TODO: to replace delta / 4 with more convenient interface
                _mm256_setr_pd(init_wave(4.0 * x + 0.00 * delta, y), 
                               init_wave(4.0 * x + 0.25 * delta, y), 
                               init_wave(4.0 * x + 0.50 * delta, y), 
                               init_wave(4.0 * x + 0.75 * delta, y)) 
        }; 
    };

    WmGeneralSolver2D<WmAvxBasicWaveStencil2D, 
                      WmGeneralConeFoldTiling2D<NTileRank>, 
                      WmAvxLinearLayer2D, NSideRank - 2> 
        solver(length, delta_time, init_func);

    solver.advance(run_count);

    return solver;
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

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
void test_wave(int argc, char* argv[], size_t run_count)
{
    if (argc < 2)
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        test(WmLogger::stream());
    }

    auto solver = run_vector<NSideRank, NTileRank>(1e2, 0.1, run_count);
    solver.layer().dump(out_stream);
}

int main(int argc, char* argv[])
{
#if defined(WM_BENCHMARK)
    run_scalar<7>(1e2, 0.1, 50);
#else
    test_wave<5>(argc, argv, 100);
#endif

    return 0;
}
