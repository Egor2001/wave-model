#ifndef WAVE_MODEL_STENCIL_AVX_AXIS_BASIC_WAVE_STENCIL2D_H_
#define WAVE_MODEL_STENCIL_AVX_AXIS_BASIC_WAVE_STENCIL2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

#include <cinttypes>
#include <iostream>

#include <immintrin.h>

namespace wave_model {

struct alignas(alignof(__m256d)) WmAvxAxisBasicWaveData2D
{
    static const __m256d VFactor;
    __m256d intencity;
};

// TODO: constexpr
const __m256d WmAvxAxisBasicWaveData2D::VFactor = _mm256_set1_pd(1.0);

template<typename TStream>
TStream& operator << (TStream& stream, 
                      const WmAvxAxisBasicWaveData2D& wave_data)
{
    thread_local double buf[4u] = {};

    _mm256_store_pd(buf, wave_data.intencity);
    stream << buf[0] << ' ' << buf[1] << ' ' << buf[2] << ' ' << buf[3];

    return stream;
}

class alignas(alignof(__m256d)) WmAvxAxisBasicWaveStencil2D
{
public:
    using TData = WmAvxAxisBasicWaveData2D;
    static constexpr size_t NDepth = 2;
    static constexpr size_t NMod = NDepth;

    static constexpr size_t NTargets = 6;

    WmAvxAxisBasicWaveStencil2D(double dspace, double dtime):
        dspace_(_mm256_set1_pd(dspace)), dtime_(_mm256_set1_pd(dtime))
    {}

    // TODO: to create enum for sides
    template<int NXSide, int NYSide, size_t NLayerIdx, typename TLayer>
    void apply(int64_t idx, TLayer* layers) const
    {
        static constexpr size_t AIdx[] = { 
            NLayerIdx % NMod, 
            (NLayerIdx + NMod - 2) % NMod, 
            (NLayerIdx + NMod - 1) % NMod
        };

        int64_t add_y = TLayer::template off_top<0>(idx, 1);
        int64_t add_x = TLayer::template off_left<0>(idx, 1);

        idx += add_x + add_y;

        // std::cerr << "APPLY(" << idx << ")\n";

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
                              TData::VFactor, 
                              _mm256_mul_pd(dtime_, inv_dspace)
                              );
        __m256d courant2 = _mm256_mul_pd(courant, courant);

        __m256d sub_x_intencity = // abcdABCD -> dABC
            _mm256_shuffle_pd(
                _mm256_permute2f128_pd(
                    layers[AIdx[1]][idx + sub_x].intencity, 
                    layers[AIdx[1]][idx].intencity, 
                    0b00'10'00'01
                    ),
                layers[AIdx[1]][idx].intencity,
                0b0101
                );

        __m256d add_x_intencity = // ABCDabcd -> BCDa
            _mm256_shuffle_pd(
                layers[AIdx[1]][idx].intencity,
                _mm256_permute2f128_pd(
                    layers[AIdx[1]][idx].intencity, 
                    layers[AIdx[1]][idx + add_x].intencity, 
                    0b00'10'00'01
                    ),
                0b0101
                );

        // TODO: to distinguish between x and y
        layers[AIdx[0]][idx] = {
            /* .intencity = */
            _mm256_sub_pd(
                _mm256_mul_pd(
                    _mm256_add_pd(
                        _mm256_sub_pd(
                            _mm256_add_pd(
                                layers[AIdx[1]][idx + add_y].intencity, 
                                layers[AIdx[1]][idx + sub_y].intencity
                                ),
                            _mm256_mul_pd(
                                _mm256_set1_pd(2.0), 
                                layers[AIdx[1]][idx].intencity
                                )
                            ),
                        _mm256_sub_pd(
                            _mm256_add_pd(add_x_intencity,
                                          sub_x_intencity),
                            _mm256_mul_pd(_mm256_set1_pd(2.0), 
                                          layers[AIdx[1]][idx].intencity)
                            )
                        ), 
                    courant2
                    ),
                layers[AIdx[2]][idx].intencity
                )
        };
    }

private:
    __m256d dspace_, dtime_;
};

} // namespace wave_model

#endif // WAVE_MODEL_STENCIL_AVX_AXIS_BASIC_WAVE_STENCIL2D_H_
