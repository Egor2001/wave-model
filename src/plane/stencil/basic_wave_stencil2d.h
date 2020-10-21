#ifndef WAVE_MODEL_STENCIL_BASIC_WAVE_STENCIL2D_H_
#define WAVE_MODEL_STENCIL_BASIC_WAVE_STENCIL2D_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

#include <cinttypes>
#include <cstdio>

namespace wave_model {

struct WmBasicWaveData2D
{
    double factor;
    double intencity;
};

template<typename TStream>
TStream& operator << (TStream& stream, const WmBasicWaveData2D& wave_data)
{
    stream << wave_data.intencity;
    return stream;
}

class WmBasicWaveStencil2D
{
public:
    using TData = WmBasicWaveData2D;
    static constexpr size_t NDepth = 2;
    static constexpr size_t NTargets = 6;

    WmBasicWaveStencil2D(double dspace, double dtime):
        dspace_(dspace), dtime_(dtime)
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
        double inv_dspace = 1.0 / dspace_;
        double courant = layers[-1][idx].factor * dtime_ * inv_dspace;
        double courant2 = courant * courant;

        // TODO: to distinguish between x and y
        layers[0][idx] = {
            /* .factor = */
                layers[-1][idx].factor,
            /* .intencity = */
                -layers[-2][idx].intencity + 
                ((layers[-1][idx + add_y].intencity + 
                  layers[-1][idx + sub_y].intencity - 
                  2.0 * layers[-1][idx].intencity) + 
                 (layers[-1][idx + add_x].intencity + 
                  layers[-1][idx + sub_x].intencity - 
                  2.0 * layers[-1][idx].intencity)
                 ) * courant2
        };

        // printf("apply to idx %#" PRIx64 "\n", idx);
    }

private:
    double dspace_, dtime_;
};

} // namespace wave_model

#endif // WAVE_MODEL_STENCIL_BASIC_WAVE_STENCIL2D_H_
