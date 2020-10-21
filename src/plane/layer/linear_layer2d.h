#ifndef WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_
#define WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_

#include "logging/macro.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace wave_model {

// TODO: border & PML
template<class TD, size_t ND>
class WmLinearLayer2D
{
public:
    struct Test;

    using TData = TD;
    static constexpr int64_t NDomainLength = ND;

    static_assert(NDomainLength && !(NDomainLength & (NDomainLength - 1)), 
                  "NDomainLength must be 2's power");

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_top([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_bottom<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_bottom([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return (NDomainLength * static_cast<int64_t>((1 << NCellRank) * cnt));
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_left([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_right<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_right([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return static_cast<int64_t>((1 << NCellRank) * cnt);
    }
    //------------------------------------------------------------ 

    WmLinearLayer2D():
        data_vec_(NDomainLength * NDomainLength)
    {}

    template<typename FInitFunc>
    void init(double length, FInitFunc func)
    {
        double scale_factor = length / NDomainLength;

        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        for (int64_t x_idx = 0; x_idx < NDomainLength; ++x_idx)
        {
            double x = (x_idx - NDomainLength / 2) * scale_factor;
            double y = (y_idx - NDomainLength / 2) * scale_factor;

            int64_t idx = y_idx * NDomainLength + x_idx;
            data_vec_[idx] = func(x, y);
        }
    }

    template<typename TStream>
    TStream& dump(TStream& stream) const noexcept
    {
        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        {
            for (int64_t x_idx = 0; x_idx < NDomainLength; ++x_idx)
                stream << data_vec_[y_idx * NDomainLength + x_idx] << ' ';

            stream << '\n';
        }

        return stream;
    }

    WmLinearLayer2D(const WmLinearLayer2D&) = delete;
    WmLinearLayer2D& operator = (const WmLinearLayer2D&) = delete;

    WmLinearLayer2D(WmLinearLayer2D&&) noexcept = default;
    WmLinearLayer2D& operator = (WmLinearLayer2D&&) noexcept = default;

    [[nodiscard]] inline TData& operator [] (int64_t idx) noexcept
    {
        WM_ASSERT(0 <= idx && idx < NDomainLength * NDomainLength, 
                  "idx is out of bounds");

        return data_vec_[idx];
    }

    [[nodiscard]] inline const TData& operator [] (int64_t idx) const noexcept
    {
        return const_cast<WmLinearLayer2D*>(this)->operator[](idx);
    }

private:
    std::vector<TData> data_vec_;
};

// TODO: to create templatized testing class for any layer type
// to avoid copy-paste
template<typename TD, size_t ND>
struct WmLinearLayer2D<TD, ND>::Test
{
    template<typename TStream>
    struct TestInitFunc
    {
        explicit TestInitFunc(TStream& stream_ref, size_t* call_cnt_ptr):
            stream(stream_ref),
            call_cnt(call_cnt_ptr)
        {}

        [[nodiscard]]
        WmLinearLayer2D::TData operator () (double x, double y) noexcept
        {
            stream << "TestInitFunc(" << x << ", " << y << ")\n";
            ++(*call_cnt);

            return {};
        }

        TStream& stream;
        size_t* call_cnt;
    };

    template<size_t NCellRank, typename TStream>
    static TStream& test_off(TStream& stream)
    {
        stream << "BEGIN WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        WM_ASSERT(WmLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                (1u << NCellRank) * WmLinearLayer2D::NDomainLength, 
                "TEST FAILED");
        WM_ASSERT(WmLinearLayer2D::off_right<NCellRank>(0, 1) == 
                (1u << NCellRank), "TEST FAILED");

        WM_ASSERT(WmLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                -WmLinearLayer2D::off_top<NCellRank>(0, 1), "TEST FAILED");
        WM_ASSERT(WmLinearLayer2D::off_right<NCellRank>(0, 1) == 
                -WmLinearLayer2D::off_left<NCellRank>(0, 1), "TEST FAILED");

        WM_ASSERT(WmLinearLayer2D::off_bottom<NCellRank>(0, 1) * 2 == 
                WmLinearLayer2D::off_bottom<NCellRank>(0, 2), "TEST FAILED");
        WM_ASSERT(WmLinearLayer2D::off_right<NCellRank>(0, 1) * 2 == 
                WmLinearLayer2D::off_right<NCellRank>(0, 2), "TEST FAILED");

        stream << "END WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmLinearLayer2D::NDomainLength * WmLinearLayer2D::NDomainLength;

        stream << "BEGIN WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_init()\n";

        WmLinearLayer2D layer;

        size_t call_cnt = 0;
        TestInitFunc init_func(stream, &call_cnt);

        layer.init(1.0, init_func);

        stream << "CALLED " << call_cnt << "\n";
        stream << "SQUARE " << NSquare << "\n";

        WM_ASSERT(call_cnt == NSquare, "TEST FAILED");

        stream << "END WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_init()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_operator(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmLinearLayer2D::NDomainLength * WmLinearLayer2D::NDomainLength;

        stream << "BEGIN WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_operator()\n";

        WmLinearLayer2D layer;
        static_cast<void>(layer[0]);
        static_cast<void>(layer[NSquare - 1]);

        stream << "END WmLinearLayer2D<$TData, " << 
            WmLinearLayer2D::NDomainLength << 
            ">::test_operator()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_
