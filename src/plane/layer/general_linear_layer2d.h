#ifndef WAVE_MODEL_GENERAL_LAYER_LINEAR_LAYER2D_H_
#define WAVE_MODEL_GENERAL_LAYER_LINEAR_LAYER2D_H_

#include "logging/macro.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace wave_model {

// TODO: border & PML
template<class TD, size_t NDX, size_t NDY = NDX>
class WmGeneralLinearLayer2D
{
public:
    struct Test;

    using TData = TD;
    static constexpr int64_t NDomainLengthX = NDX;
    static constexpr int64_t NDomainLengthY = NDY;

    static_assert(NDomainLengthX && !(NDomainLengthX & (NDomainLengthX - 1)), 
                  "NDomainLengthX must be 2's power");

    static_assert(NDomainLengthY && !(NDomainLengthY & (NDomainLengthY - 1)), 
                  "NDomainLengthY must be 2's power");

    static_assert(NDomainLengthX <= NDomainLengthY,
                  "NDomainLengthX must be not greater than NDomainLengthY");

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_top([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_bottom<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_bottom([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return (NDomainLengthX * static_cast<int64_t>((1 << NCellRank) * cnt));
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

    WmGeneralLinearLayer2D():
        data_vec_(NDomainLengthY * NDomainLengthX)
    {}

    template<typename FInitFunc>
    void init(double length, FInitFunc func)
    {
        double scale_factor = length / NDomainLengthX;

        for (int64_t y_idx = 0; y_idx < NDomainLengthY; ++y_idx)
        for (int64_t x_idx = 0; x_idx < NDomainLengthX; ++x_idx)
        {
            double x = scale_factor * 
                static_cast<double>(x_idx - NDomainLengthX / 2);
            double y = scale_factor * 
                static_cast<double>(y_idx - NDomainLengthY / 2);

            int64_t idx = y_idx * NDomainLengthY + x_idx;
            data_vec_[idx] = func(x, y);
        }
    }

    template<typename TStream>
    TStream& dump(TStream& stream) const noexcept
    {
        for (int64_t y_idx = 0; y_idx < NDomainLengthY; ++y_idx)
        {
            for (int64_t x_idx = 0; x_idx < NDomainLengthX; ++x_idx)
                stream << data_vec_[y_idx * NDomainLengthY + x_idx] << ' ';

            stream << '\n';
        }

        return stream;
    }

    WmGeneralLinearLayer2D
        (const WmGeneralLinearLayer2D&) = delete;
    WmGeneralLinearLayer2D& operator = 
        (const WmGeneralLinearLayer2D&) = delete;

    WmGeneralLinearLayer2D
        (WmGeneralLinearLayer2D&&) noexcept = default;
    WmGeneralLinearLayer2D& operator = 
        (WmGeneralLinearLayer2D&&) noexcept = default;

    [[nodiscard]] inline TData& operator [] (int64_t idx) noexcept
    {
        WM_ASSERT(0 <= idx && idx < NDomainLengthY * NDomainLengthX, 
                  "idx is out of bounds");

        return data_vec_[idx];
    }

    [[nodiscard]] inline const TData& operator [] (int64_t idx) const noexcept
    {
        return const_cast<WmGeneralLinearLayer2D*>(this)->operator[](idx);
    }

private:
    std::vector<TData> data_vec_;
};

// TODO: to create templatized testing class for any layer type
// to avoid copy-paste
template<typename TD, size_t NDX, size_t NDY>
struct WmGeneralLinearLayer2D<TD, NDX, NDY>::Test
{
    template<typename TStream>
    struct TestInitFunc
    {
        explicit TestInitFunc(TStream& stream_ref, size_t* call_cnt_ptr):
            stream(stream_ref),
            call_cnt(call_cnt_ptr)
        {}

        [[nodiscard]]
        WmGeneralLinearLayer2D::TData operator () (double x, double y) noexcept
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
        stream << "BEGIN WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_off<" << NCellRank << ">()\n";

        WM_ASSERT(WmGeneralLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                (1u << NCellRank) * WmGeneralLinearLayer2D::NDomainLengthY, 
                "TEST FAILED");
        WM_ASSERT(WmGeneralLinearLayer2D::off_right<NCellRank>(0, 1) == 
                (1u << NCellRank), "TEST FAILED");

        WM_ASSERT(WmGeneralLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                  -WmGeneralLinearLayer2D::off_top<NCellRank>(0, 1), 
                  "TEST FAILED");
        WM_ASSERT(WmGeneralLinearLayer2D::off_right<NCellRank>(0, 1) == 
                  -WmGeneralLinearLayer2D::off_left<NCellRank>(0, 1), 
                  "TEST FAILED");

        WM_ASSERT(WmGeneralLinearLayer2D::off_bottom<NCellRank>(0, 1) * 2 == 
                  WmGeneralLinearLayer2D::off_bottom<NCellRank>(0, 2), 
                  "TEST FAILED");
        WM_ASSERT(WmGeneralLinearLayer2D::off_right<NCellRank>(0, 1) * 2 == 
                  WmGeneralLinearLayer2D::off_right<NCellRank>(0, 2), 
                  "TEST FAILED");

        stream << "END WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_off<" << NCellRank << ">()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmGeneralLinearLayer2D::NDomainLengthY * 
            WmGeneralLinearLayer2D::NDomainLengthX;

        stream << "BEGIN WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_init()\n";

        WmGeneralLinearLayer2D layer;

        size_t call_cnt = 0;
        TestInitFunc<TStream> init_func(stream, &call_cnt);

        layer.init(1.0, init_func);

        stream << "CALLED " << call_cnt << "\n";
        stream << "SQUARE " << NSquare << "\n";

        WM_ASSERT(call_cnt == NSquare, "TEST FAILED");

        stream << "END WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_init()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_operator(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmGeneralLinearLayer2D::NDomainLengthY * 
            WmGeneralLinearLayer2D::NDomainLengthX;

        stream << "BEGIN WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_operator()\n";

        WmGeneralLinearLayer2D layer;
        static_cast<void>(layer[0]);
        static_cast<void>(layer[NSquare - 1]);

        stream << "END WmGeneralLinearLayer2D<$TData, " << 
            WmGeneralLinearLayer2D::NDomainLengthX << ", " <<
            WmGeneralLinearLayer2D::NDomainLengthY << 
            ">::test_operator()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_GENERAL_LAYER_LINEAR_LAYER2D_H_
