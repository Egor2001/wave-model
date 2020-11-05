// #define WM_BENCHMARK

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

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
#include <memory>

/// @brief
using namespace wave_model;

/**
 * @brief Runs scalar computations
 *
 * Properties:
 * - Solver: general
 * - Stencil: Basic 2-order scalar
 * - Data: Z-order
 * - Tiling: ConeFold
 * - Initial: Cosine hat
 *
 * @tparam NSideRank Rank of the domain side
 * @tparam NTileRank Rank of the tiling depth
 * @param length Domain length
 * @param delta_time Time discretization delta
 * @param run_count Number of layer calculation steps
 */
template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_scalar(double length, double delta_time, size_t run_count)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            // .intencity = 
            init_wave(x, y) 
        }; 
    };

    auto solver = 
        std::make_unique<
            WmGeneralSolver2D<
                WmBasicWaveStencil2D, 
                WmGeneralConeFoldTiling2D<
                    NTileRank
                    >, 
                WmGeneralZCurveLayer2D, 
                NSideRank, 
                NSideRank
                > 
            >
        (length, delta_time, init_func);

    solver->advance(run_count);

    return solver;
}

/**
 * @brief Runs vectorized-by-axis computations
 *
 * Properties:
 * - Solver: general
 * - Stencil: Basic 2-order vectorized-by-axis with AVX
 * - Data: Linear
 * - Tiling: ConeFold
 * - Initial: Cosine hat
 *
 * @tparam NSideRank Rank of the domain side
 * @tparam NTileRank Rank of the tiling depth
 * @param length Domain length
 * @param delta_time Time discretization delta
 * @param run_count Number of layer calculation steps
 */
template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_vector_axis(double length, double delta_time, size_t run_count)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    double delta = length / (1u << (NSideRank - 2));
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave, delta](double x, double y) -> 
        WmAvxAxisBasicWaveData2D
    { 
        return { 
            // .intencity = 
            // TODO: to replace delta / 4 with more convenient interface
                _mm256_setr_pd(init_wave(4.0 * x + 0.00 * delta, y), 
                               init_wave(4.0 * x + 0.25 * delta, y), 
                               init_wave(4.0 * x + 0.50 * delta, y), 
                               init_wave(4.0 * x + 0.75 * delta, y)) 
        }; 
    };

    auto solver = 
        std::make_unique<
            WmGeneralSolver2D<
                WmAvxAxisBasicWaveStencil2D, 
                WmGeneralConeFoldTiling2D<
                    NTileRank
                    >, 
                WmGeneralLinearLayer2D, 
                NSideRank - 2, 
                NSideRank
                >
            >
        (length, delta_time, init_func);

    solver->advance(run_count);

    return solver;
}

/**
 * @brief Runs vectorized-by-quad computations
 *
 * Properties:
 * - Solver: general
 * - Stencil: Basic 2-order vectorized-by-quad with AVX
 * - Data: Linear
 * - Tiling: ConeFold
 * - Initial: Cosine hat
 *
 * @tparam NSideRank Rank of the domain side
 * @tparam NTileRank Rank of the tiling depth
 * @param length Domain length
 * @param delta_time Time discretization delta
 * @param run_count Number of layer calculation steps
 */
template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_vector_quad(double length, double delta_time, size_t run_count)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    double delta = length / (1u << (NSideRank - 1));
    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave, delta](double x, double y) -> 
        WmAvxQuadBasicWaveData2D
    { 
        return { 
            // .intencity = 
            // TODO: to replace delta / 4 with more convenient interface
                _mm256_setr_pd(init_wave(x - 0.5 * delta, y - 0.5 * delta), 
                               init_wave(x + 0.5 * delta, y - 0.5 * delta), 
                               init_wave(x - 0.5 * delta, y + 0.5 * delta), 
                               init_wave(x + 0.5 * delta, y + 0.5 * delta)) 
        }; 
    };

    auto solver = 
        std::make_unique<
            WmGeneralSolver2D<
                WmAvxQuadBasicWaveStencil2D, 
                WmGeneralConeFoldTiling2D<
                    NTileRank
                    >, 
                WmGeneralLinearLayer2D, 
                NSideRank - 1, 
                NSideRank - 1
                > 
            >
        (length, delta_time, init_func);

    solver->advance(run_count);

    return solver;
}

/**
 * @brief Runs distributed-grid computations
 *
 * Properties:
 * - Solver: parallel
 * - Stencil: Basic 2-order vectorized-by-quad with AVX
 * - Data: Linear
 * - Tiling: ConeFold
 * - Initial: Cosine hat
 *
 * @tparam NSideRank Rank of the domain side
 * @tparam NTileRank Rank of the tiling depth
 * @param length Domain length
 * @param delta_time Time discretization delta
 * @param run_count Number of layer calculation steps
 */
template<size_t NSideRank, size_t NTileRank = NSideRank - 2>
auto run_parallel(double length, double delta_time, size_t run_count)
{
    static_assert(!(NSideRank < NTileRank), "side must not be less than tile");

    WmCosineHatWave2D init_wave { /* .ampl = */ 1.0, /* .freq = */ 0.5 };
    auto init_func = [&init_wave](double x, double y) -> WmBasicWaveData2D
    { 
        return { 
            // .intencity = 
            init_wave(x, y) 
        }; 
    };

    auto solver = 
        std::make_unique<
            WmParallelSolver2D<
                WmConeFoldGrid2D<
                WmGeneralZCurveLayer2D<
                    WmBasicWaveData2D, 
                    NSideRank
                    >, 
                WmBasicWaveStencil2D, 
                WmGeneralConeFoldTiling2D<NTileRank>, 
                NTileRank>
                >
            >
        (length, delta_time, init_func);

    // TODO: to fix parallel execution
    WmThreadPoolExecutor executor(4);
    // WmSequentialExecutor executor;

    solver->advance(executor, run_count);

    return solver;
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

template<typename TStream>
TStream& test_parallel(TStream& stream)
{
    wm_test_thread_pool_executor(stream);

    return stream;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
#if !defined(WM_BENCHMARK)
    if (argc < 2)
        std::cerr << "USAGE: " << argv[0] << " OUTFILE [LOGFILE]\n";

    std::ofstream out_stream(argv[1]);
    if (argc > 2)
    {
        WmLogger::init(argv[2]);
        // test(WmLogger::stream());
    }

    static constexpr size_t NSideRank = 8;
    static constexpr size_t NTileRank = 4;
    static constexpr size_t NRunCount = 500;

#else // defined(WM_BENCHMARK)
    static constexpr size_t NSideRank = 12;
    static constexpr size_t NTileRank = 3;
    static constexpr size_t NRunCount = 500;

#endif // defined(WM_BENCHMARK)

    // auto solver = run_scalar     <NSideRank, NTileRank>(1e2, 0.1, NRunCount);
    auto solver = run_parallel   <NSideRank, NTileRank>(1e2, 0.1, NRunCount);
    // auto solver = run_vector_quad<NSideRank, NTileRank>(1e2, 0.1, NRunCount);
    // auto solver = run_vector_axis<NSideRank, NTileRank>(1e2, 0.1, NRunCount);

#if !defined(WM_BENCHMARK)
    solver->layer().dump(out_stream);

#endif // defined(WM_BENCHMARK)

    return 0;
}
