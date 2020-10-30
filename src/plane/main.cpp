#define WM_BENCHMARK

#include "test/parallel/thread_pool_executor_test.h"
#include "parallel/thread_pool_executor.h"
#include "parallel/sequential_executor.h"
#include "parallel/conefold_node2d.h"
#include "parallel/conefold_grid2d.h"

#include "general_solver2d.h"
#include "parallel_solver2d.h"
#include "logging/macro.h"
#include "logging/logger.h"

#include "layer/general_linear_layer2d.h"
#include "layer/general_zcurve_layer2d.h"
#include "stencil/basic_wave_stencil2d.h"
#include "stencil/avx_axis_basic_wave_stencil2d.h"
#include "stencil/avx_quad_basic_wave_stencil2d.h"
#include "tiling/general_conefold_tiling2d.h"

#include "wave/cosine_hat_wave2d.h"
#include "wave/gaussian_wave2d.h"

/*
#include "test/tiling/conefold_tiling2d_test.h"
#include "test/layer/linear_layer2d_test.h"
#include "test/layer/zcurve_layer2d_test.h"
*/

#include <iostream>
#include <fstream>

using namespace wave_model;

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_scalar(double length /* = 1e2 */, 
                double delta_time /* = 0.1 */, 
                size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            /* .intencity = */ init_wave(x, y) 
        }; 
    };

    WmGeneralSolver2D<WmBasicWaveStencil2D, 
                      WmGeneralConeFoldTiling2D<NTileRank>, 
                      WmGeneralZCurveLayer2D, NSideRank, NSideRank> 
        solver(length, delta_time, init_func);

    solver.advance(run_count);

    return solver;
}

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_vector_axis(double length /* = 1e2 */, 
                     double delta_time /* =  0/1 */, 
                     size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    double delta = length / (1u << (NSideRank - 2));
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave, delta](double x, double y) -> 
        WmAvxAxisBasicWaveData2D
    { 
        // std::cerr << "FUNC (" << x << ", " << y << ")\n";

        return { 
            // .intencity = 
            // TODO: to replace delta / 4 with more convenient interface
                _mm256_setr_pd(init_wave(4.0 * x + 0.00 * delta, y), 
                               init_wave(4.0 * x + 0.25 * delta, y), 
                               init_wave(4.0 * x + 0.50 * delta, y), 
                               init_wave(4.0 * x + 0.75 * delta, y)) 
        }; 
    };

    WmGeneralSolver2D<WmAvxAxisBasicWaveStencil2D, 
                      WmGeneralConeFoldTiling2D<NTileRank>, 
                      WmGeneralLinearLayer2D, NSideRank - 2, NSideRank> 
        solver(length, delta_time, init_func);

    solver.advance(run_count);

    return solver;
}

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_vector_quad(double length /* = 1e2 */, 
                     double delta_time /* =  0/1 */, 
                     size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    double delta = length / (1u << (NSideRank - 1));
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave, delta](double x, double y) -> 
        WmAvxQuadBasicWaveData2D
    { 
        // std::cerr << "FUNC (" << x << ", " << y << ")\n";

        return { 
            // .intencity = 
            // TODO: to replace delta / 4 with more convenient interface
                _mm256_setr_pd(init_wave(x - 0.5 * delta, y - 0.5 * delta), 
                               init_wave(x + 0.5 * delta, y - 0.5 * delta), 
                               init_wave(x - 0.5 * delta, y + 0.5 * delta), 
                               init_wave(x + 0.5 * delta, y + 0.5 * delta)) 
        }; 
    };

    WmGeneralSolver2D<WmAvxQuadBasicWaveStencil2D, 
                      WmGeneralConeFoldTiling2D<NTileRank>, 
                      WmGeneralLinearLayer2D, NSideRank - 1, NSideRank - 1> 
        solver(length, delta_time, init_func);

    solver.advance(run_count);

    return solver;
}

template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
void run_parallel(double length /* = 1e2 */, 
                  double delta_time /* = 0.1 */, 
                  size_t run_count /* = 1 << ((15 - NSideRank) * 2) */)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            /* .intencity = */ init_wave(x, y) 
        }; 
    };

    WmParallelSolver2D<
        WmConeFoldGrid2D<
            WmGeneralZCurveLayer2D<WmBasicWaveData2D, 1u << NSideRank>, 
            WmBasicWaveStencil2D, 
            WmGeneralConeFoldTiling2D<NTileRank>, NTileRank>>
        solver(length, delta_time, init_func);

    WmThreadPoolExecutor executor(4);
    // WmSequentialExecutor executor;

    // TODO: to fix node class before switching to sequential execution
    solver.advance(executor, run_count);
}

/*
template<typename TStream>
TStream& test(TStream& stream)
{
    wm_test_linear_layer2d(stream);
    wm_test_zcurve_layer2d(stream);

    wm_test_conefold_tiling2d<WmLinearLayer2D>(stream);
    wm_test_conefold_tiling2d<WmZCurveLayer2D>(stream);

    return stream;
}
*/

// TODO: return unique_ptr
template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
void test_wave(int argc, char* argv[], size_t run_count)
{
    if (argc < 2)
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        // test(WmLogger::stream());
    }

    auto solver = run_vector_quad<NSideRank, NTileRank>(1e2, 0.1, run_count);
    solver.layer().dump(out_stream);
}

template<typename TStream>
TStream& test_parallel(TStream& stream)
{
    wm_test_thread_pool_executor(stream);

    return stream;
}

int main([[maybe_unused]] int argc, 
         [[maybe_unused]] char* argv[])
{
    // test_parallel(std::cerr);

#if defined(WM_BENCHMARK)
    run_parallel<5, 3>(1e2, 0.1, 1/* 500 */);
    // run_vector_quad<12, 3>(1e2, 0.1, 500);
#else
    test_wave<7>(argc, argv, 100);
#endif

    return 0;
}
