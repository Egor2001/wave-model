#ifndef WAVE_MODEL_CORE_H
#define WAVE_MODEL_CORE_H

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

// master test function
void test_core(double factor, double dspace);

namespace {

enum class WmSide
{
    SIDE_UP, SIDE_DN, SIDE_LT, SIDE_RT, 
    SIDE_CNT = 4
};

struct WmTarget2D
{
    size_t depth;
    int64_t x_diff, y_diff;
};

template<class T, size_t N>
class WmLayer2D
{
public:
    using TData = T;
    static constexpr uint64_t NSize = N;

    WmLayer2D():
        data_vec_(NSize * NSize)
    {}

    template<class FInitFunc>
    explicit WmLayer2D(FInitFunc func):
        WmLayer2D()
    {
        for (uint64_t idx = 0; idx < NSize * NSize; ++idx)
        {
            uint64_t y_idx = idx / NSize;
            uint64_t x_idx = idx % NSize;

            set(idx, func(x_idx, y_idx));
        }
    }

    WmLayer2D(const WmLayer2D&) = delete;
    WmLayer2D& operator = (const WmLayer2D&) = delete;

    WmLayer2D(WmLayer2D&&) noexcept = default;
    WmLayer2D& operator = (WmLayer2D&&) noexcept = default;

    // TODO: to duplicate for rvalues
    void set(uint64_t idx, const TData& data)
    {
        data_vec_[idx] = data;
    }

    const TData& get(uint64_t idx) const
    {
        return data_vec_[idx];
    }

    const TData& get(uint64_t idx, int64_t off_x, int64_t off_y) const
    {
        int64_t y_idx = (idx / NSize) + off_y;
        int64_t x_idx = (idx % NSize) + off_x;

        if (x_idx < 0)
            x_idx = -x_idx % NSize;

        if (x_idx >= static_cast<int64_t>(NSize)) 
            x_idx = (NSize - 1) - (x_idx % NSize);

        if (y_idx < 0)
            y_idx = -y_idx % NSize;

        if (y_idx >= static_cast<int64_t>(NSize))
            y_idx = (NSize - 1) - (y_idx % NSize);

        return data_vec_[y_idx * NSize + x_idx];
    }

private:
    std::vector<TData> data_vec_;
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
    static constexpr WmTarget2D ATargets[] = {
        { .depth = 1u, .x_diff = 0,  .y_diff = 0 },
        { .depth = 1u, .x_diff = 1,  .y_diff = 0 },
        { .depth = 1u, .x_diff = -1, .y_diff = 0 },
        { .depth = 1u, .x_diff = 0,  .y_diff = 1 },
        { .depth = 1u, .x_diff = 0,  .y_diff = -1 },
        { .depth = 2u, .x_diff = 0,  .y_diff = 0 }
    };

    WmBasicWaveStencil(double dspace, double dtime, size_t accuracy = 2):
        dspace_(dspace), dtime_(dtime),
        accuracy_(accuracy)
    {}

    // TODO: to restrict to the RandomAccessIterator
    template<typename TLayerIter>
    TData apply(uint64_t idx, const TLayerIter& layers_begin)
    {
        using TLayer = typename std::iterator_traits<TLayerIter>::value_type;
        static_assert(TLayer::NSize > 0, "TLayer::NSize must be nonzero");
        static_assert(std::is_same_v<typename TLayer::TData, WmBasicWaveData>, 
                      "TLayer::TData must be WmBasicWaveData");

        const TLayer& ref_prev = *layers_begin;
        const TLayer& ref_prev_prev = *(layers_begin + 1);

        // TODO: to extend up to accuracy = 4
        // TODO: to dittinguish between x and y
        double inv_dspace = 1.0 / dspace_;
        double courant = ref_prev.get(idx, 0, 0).factor * dtime_ * inv_dspace;
        double courant2 = courant * courant;

        // TODO: to distinguish between x and y
        TData result = {
            .factor = 
                ref_prev.get(idx, 0, 0).factor,
            .intencity = 
                -ref_prev_prev.get(idx, 0, 0).intencity + 
                ((ref_prev.get(idx, 0, 1).intencity + 
                  ref_prev.get(idx, 0, -1).intencity - 
                  2.0 * ref_prev.get(idx, 0, 0).intencity) + 
                 (ref_prev.get(idx, 1, 0).intencity + 
                  ref_prev.get(idx, -1, 0).intencity - 
                  2.0 * ref_prev.get(idx, 0, 0).intencity)
                 ) * courant2
        };

        return result;
    }

private:
    double dspace_, dtime_;
    size_t accuracy_;
};

template<class T, size_t N>
class WmSolver
{
public:
    using TStencil = T;
    static constexpr uint64_t NSize = N;

    using TLayer = WmLayer2D<typename TStencil::TData, NSize>;

    // TODO: to duplicate for rvalues
    WmSolver(const TStencil& stencil, TLayer&& init_state):
        stencil_(stencil), layers_vec_(1 + TStencil::NDepth)
    {
        layers_vec_.front() = std::move(init_state);
    }

    WmSolver(const WmSolver&) = delete;
    WmSolver& operator = (const WmSolver&) = delete;

    void advance(uint64_t times)
    {
        for (uint64_t time_idx = 0; time_idx < times; ++time_idx)
        {
            // rotate right by 1
            std::rotate(std::rbegin(layers_vec_), std::rbegin(layers_vec_) + 1,
                        std::rend(layers_vec_));

            for (size_t idx = 0; idx < NSize * NSize; ++idx)
            {
                layers_vec_.front().set(idx, 
                        stencil_.apply(idx, std::begin(layers_vec_) + 1));
            }
        }
    }

    const TLayer& get_layer() const
    {
        return layers_vec_.front();
    }

private:
    TStencil stencil_;
    std::vector<TLayer> layers_vec_;
};

} // namespace

#endif // WAVE_MODEL_CORE_H
