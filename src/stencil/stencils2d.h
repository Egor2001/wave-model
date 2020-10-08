#ifndef WAVE_MODEL_STENCIL_STENCILS2D_H_
#define WAVE_MODEL_STENCIL_STENCILS2D_H_

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace stencils {

struct WmTarget2D
{
    uint64_t depth;
    int64_t x_diff, y_diff;
};

struct WmBasicWaveData
{
    double factor;
    double intencity;
};

class WmBasicWaveStencil
{
public:
    using TData = WmBasicWaveData;
    static constexpr uint64_t NDepth = 2;
    static constexpr NTargets = 6;
    static constexpr std::array<WmTarget2D, NTargets> ATargets { {
            { .depth = 1u, .x_diff = 0,  .y_diff = 0 },
            { .depth = 1u, .x_diff = 1,  .y_diff = 0 },
            { .depth = 1u, .x_diff = -1, .y_diff = 0 },
            { .depth = 1u, .x_diff = 0,  .y_diff = 1 },
            { .depth = 1u, .x_diff = 0,  .y_diff = -1 },
            { .depth = 2u, .x_diff = 0,  .y_diff = 0 }
        }
    };

    template<uint64_t Depth, int64_t XDiff, int64_t YDiff, int64_t Idx>
    static constexpr int64_t get_helper() noexcept
    {
        return (((ATargets[Idx - 1].depth == Depth && 
                  ATargets[Idx - 1].x_diff == XDiff && 
                  ATargets[Idx - 1].y_diff == YDiff) || Idx == 0) ? 
                Idx - 1 : get_helper<Depth, XDiff, YDiff, Idx - 1>());
    }

    template<uint64_t Depth, int64_t XDiff, int64_t YDiff>
    static constexpr const TData& 
    get(const std::array<TData, NTargets>& data_arr) noexcept
    {
        return std::get<get_helper<Depth, XDiff, YDiff, NTargets>>(data_arr);
    }

    WmBasicWaveStencil(double dspace, double dtime):
        dspace_(dspace), dtime_(dtime)
    {}

    TData apply(const std::array<TData, NTargets>& data_arr)
    {
        // TODO: to dittinguish between x and y
        double inv_dspace = 1.0 / dspace_;
        double courant = get<1u, 0, 0>(data_arr).factor * dtime_ * inv_dspace;
        double courant2 = courant * courant;

        // TODO: to distinguish between x and y
        TData result = {
            .factor = 
                get<1u, 0, 0>(data_arr).factor,
            .intencity = 
                -get<2u, 0, 0>(data_arr).intencity + 
                ((get<1u, 0, 1>(data_arr).intencity + 
                  get<1u, 0, -1>(data_arr).intencity - 
                  2.0 * get<1u, 0, 0>(data_arr).intencity) + 
                 (get<1u, 1, 0>(data_arr).intencity + 
                  get<1u, -1, 0>(data_arr).intencity - 
                  2.0 * get<1u, 0, 0>(data_arr).intencity)
                 ) * courant2
        };

        return result;
    }

private:
    double dspace_, dtime_;
};

} // namespace

#endif // WAVE_MODEL_STENCIL_STENCILS2D_H_
