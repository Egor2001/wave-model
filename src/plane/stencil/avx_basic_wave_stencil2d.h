#ifndef WAVE_MODEL_STENCIL_AVX_BASIC_WAVE_STENCIL2D_H_
#define WAVE_MODEL_STENCIL_AVX_BASIC_WAVE_STENCIL2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

#include <cinttypes>
#include <cstdio>

#include <immintrin.h>

namespace wave_model {

struct alignas(alignof(__m256d)) WmAvxBasicWaveData2D
{
    __m256d factor;
    __m256d intencity;
};

template<typename TStream>
TStream& operator << (TStream& stream, const WmAvxBasicWaveData2D& wave_data)
{
    thread_local double buf[4u] = {};

    _mm256_store_pd(buffer, wave_data.intencity);
    stream << buf[0] << ' ' << buf[1] << ' ' << buf[2] << ' ' << buf[3];

    return stream;
}

class alignas(alignof(__m256d)) WmAvxBasicWaveStencil2D
{
public:
    using TData = WmAvxBasicWaveData2D;
    static constexpr size_t NDepth = 2;
    static constexpr size_t NTargets = 6;

    WmAvxBasicWaveStencil2D(double dspace, double dtime):
        dspace_(_mm256_set1_pd(dspace)), dtime_(_mm256_set1_pd(dtime))
    {}

    // TODO: to create enum for sides
    template<int NXSide, int NYSide, typename TLayer>
    void apply(int64_t idx, TLayer* layers) const
    {
        int64_t add_y = TLayer::template off_top<0>(idx, 1);
        int64_t add_x = TLayer::template off_left<0>(idx, 1);

        idx += add_x + add_y;

        // TODO: to compare with enum
        if constexpr (NXSide > 0) add_x = 0;
        else add_x = -add_x;

        if constexpr (NYSide > 0) add_y = 0;
        else add_y = -add_y;

        int64_t sub_x = 0;
        int64_t sub_y = 0;

        // TODO: to compare with enum
        if constexpr (NXSide >= 0) 
            sub_x = TLayer::template off_left<0>(idx, 1);

        if constexpr (NYSide >= 0) 
            sub_y = TLayer::template off_top<0>(idx, 1);

        // TODO: to dittinguish between x and y
        __m256d inv_dspace = _mm256_div_pd(_mm256_set1_pd(1.0), dspace_);
        __m256d courant = _mm256_mul_pd(
                              layers[-1][idx].factor, 
                              _mm256_mul_pd(dtime_, inv_dspace)
                              );
        __m256d courant2 = _mm256_mul_pd(courant, courant);

        __m256d sub_x_intencity = // abcdABCD -> dABC
            _mm256_shuffle_pd(
                _mm256_permute2f128_pd(
                    layers[-1][idx + sub_x].intencity, 
                    layers[-1][idx].intencity, 
                    0b00'10'00'01
                    ),
                layers[-1][idx].intencity,
                0b0101
                );

        __m256d add_x_intencity = // ABCDabcd -> BCDa
            _mm256_shuffle_pd(
                layers[-1][idx].intencity,
                _mm256_permute2f128_pd(
                    layers[-1][idx].intencity, 
                    layers[-1][idx + add_x].intencity, 
                    0b00'10'00'01
                    ),
                0b0101
                );

        // TODO: to distinguish between x and y
        layers[0][idx] = {
            /* .factor = */
            layers[-1][idx].factor,
            /* .intencity = */
            _mm256_sub_pd(
                _mm256_mul_pd(
                    _mm256_add_pd(
                        _mm256_sub_pd(
                            _mm256_add_pd(layers[-1][idx + add_y].intencity,
                                          layers[-1][idx + sub_y].intencity),
                            _mm256_mul_pd(_mm256_set1_pd(2.0), 
                                          layers[-1][idx].intencity)
                            ),
                        _mm256_sub_pd(
                            _mm256_add_pd(add_x_intencity,
                                          sub_x_intencity),
                            _mm256_mul_pd(_mm256_set1_pd(2.0), 
                                          layers[-1][idx].intencity)
                            )
                        ), 
                    courant2
                    ),
                layers[-2][idx].intencity
                )
        };
    }

private:
    __m256d dspace_, dtime_;
};

} // namespace wave_model

#endif // WAVE_MODEL_STENCIL_AVX_BASIC_WAVE_STENCIL2D_H_
