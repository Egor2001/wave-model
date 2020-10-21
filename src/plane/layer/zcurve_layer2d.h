#ifndef WAVE_MODEL_LAYER_ZCURVE_LAYER_H_
#define WAVE_MODEL_LAYER_ZCURVE_LAYER_H_

#include "logging/macro.h"

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace wave_model {

// TODO: border & PML
template<class TD, size_t ND>
class WmZCurveLayer2D
{
public:
    struct Test;

    using TData = TD;
    static constexpr int64_t NDomainLength = ND;

    static_assert(NDomainLength && !(NDomainLength & (NDomainLength - 1)), 
                  "NDomainLength must be 2's power");

    // '01010101' x 8
    static constexpr uint64_t ZOrderMask = 0x5555555555555555;
    // Z-order sequence first 8 values 
    static constexpr uint64_t ZOffsetArr[] = { 0, 1, 4, 5, 16, 17, 20, 21 };
    static constexpr uint64_t ZOffsetArrSize = 
        sizeof(ZOffsetArr) / sizeof(ZOffsetArr[0]);

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr inline
    int64_t off_top(uint64_t idx, uint64_t cnt) noexcept
    {
        constexpr uint64_t NOffset = (1 << NCellRank) * (1 << NCellRank);
        WM_ASSERT(cnt < ZOffsetArrSize, "cnt is too big");

        return static_cast<int64_t>(
                ((idx & (ZOrderMask << 1)) - 
                 2 * NOffset * ZOffsetArr[cnt]) & (ZOrderMask << 1)
                ) - static_cast<int64_t>(idx & (ZOrderMask << 1));
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr inline
    int64_t off_bottom(uint64_t idx, uint64_t cnt) noexcept
    {
        constexpr uint64_t NOffset = (1 << NCellRank) * (1 << NCellRank);
        WM_ASSERT(cnt < ZOffsetArrSize, "cnt is too big");

        return static_cast<int64_t>(
                ((idx | ZOrderMask) + 
                 2 * NOffset * ZOffsetArr[cnt]) & (ZOrderMask << 1)
                ) - static_cast<int64_t>(idx & (ZOrderMask << 1));
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr inline
    int64_t off_left(uint64_t idx, size_t cnt) noexcept
    {
        constexpr uint64_t NOffset = (1 << NCellRank) * (1 << NCellRank);
        WM_ASSERT(cnt < ZOffsetArrSize, "cnt is too big");

        return static_cast<int64_t>(
                ((idx & ZOrderMask) - 
                 NOffset * ZOffsetArr[cnt]) & ZOrderMask
                ) - static_cast<int64_t>(idx & ZOrderMask);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr inline
    int64_t off_right(uint64_t idx, size_t cnt) noexcept
    {
        constexpr uint64_t NOffset = (1 << NCellRank) * (1 << NCellRank);
        WM_ASSERT(cnt < ZOffsetArrSize, "cnt is too big");

        return static_cast<int64_t>(
                ((idx | (ZOrderMask << 1)) + 
                 NOffset * ZOffsetArr[cnt]) & ZOrderMask
                ) - static_cast<int64_t>(idx & ZOrderMask);
    }
    //------------------------------------------------------------ 

    WmZCurveLayer2D():
        data_vec_(NDomainLength * NDomainLength)
    {}

    template<typename FInitFunc>
    void init(double length, FInitFunc func)
    {
        double scale_factor = length / NDomainLength;

        int64_t row_idx = 0;
        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        {
            int64_t idx = row_idx;
            for (int64_t x_idx = 0; x_idx < NDomainLength; ++x_idx)
            {
                double x = scale_factor * 
                    static_cast<double>(x_idx - NDomainLength / 2);
                double y = scale_factor * 
                    static_cast<double>(y_idx - NDomainLength / 2);

                WM_ASSERT(0 <= idx && idx < NDomainLength * NDomainLength,
                          "idx is out of bounds");

                data_vec_[idx] = func(x, y);

                idx += off_right<0>(idx, 1);
            }

            row_idx += off_bottom<0>(row_idx, 1);
        }
    }

    template<typename TStream>
    TStream& dump(TStream& stream) const noexcept
    {
        int64_t row_idx = 0;
        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        {
            int64_t idx = row_idx;
            for (int64_t x_idx = 0; x_idx < NDomainLength; ++x_idx)
            {
                stream << data_vec_[idx] << ' ';
                idx += off_right<0>(idx, 1);
            }

            stream << '\n';
            row_idx += off_bottom<0>(row_idx, 1);
        }

        return stream;
    }

    WmZCurveLayer2D(const WmZCurveLayer2D&) = delete;
    WmZCurveLayer2D& operator = (const WmZCurveLayer2D&) = delete;

    WmZCurveLayer2D(WmZCurveLayer2D&&) noexcept = default;
    WmZCurveLayer2D& operator = (WmZCurveLayer2D&&) noexcept = default;

    [[nodiscard]] inline TData& operator [] (int64_t idx)
    {
        WM_ASSERT(0 <= idx && idx < NDomainLength * NDomainLength,
                  "idx is out of bounds");

        return data_vec_[idx];
    }

    [[nodiscard]] inline const TData& operator [] (int64_t idx) const
    {
        return const_cast<WmZCurveLayer2D*>(this)->operator[](idx);
    }

private:
    std::vector<TData> data_vec_;
};

// TODO: to create templatized testing class for any layer type
// to avoid copy-paste
template<typename TD, size_t ND>
struct WmZCurveLayer2D<TD, ND>::Test
{
    template<typename TStream>
    struct TestInitFunc
    {
        explicit TestInitFunc(TStream& stream_ref, size_t* call_cnt_ptr):
            stream(stream_ref),
            call_cnt(call_cnt_ptr)
        {}

        [[nodiscard]]
        WmZCurveLayer2D::TData operator () (double x, double y) noexcept
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
        stream << "BEGIN WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        static constexpr size_t NCnt = WmZCurveLayer2D::ZOffsetArrSize / 4;

        // TODO: to refactor with static_assert
        
        [[maybe_unused]]
        int64_t x_inc = WmZCurveLayer2D::off_right<NCellRank>(1, NCnt);
        [[maybe_unused]]
        int64_t x_dec = WmZCurveLayer2D::off_left<NCellRank>(1 + x_inc, NCnt);
        WM_ASSERT(x_inc + x_dec == 0, "TEST FAILED");

        [[maybe_unused]]
        int64_t y_inc = WmZCurveLayer2D::off_bottom<NCellRank>(1, NCnt);
        [[maybe_unused]]
        int64_t y_dec = WmZCurveLayer2D::off_top<NCellRank>(1 + y_inc, NCnt);
        WM_ASSERT(y_inc + y_dec == 0, "TEST FAILED");

        [[maybe_unused]]
        int64_t x_dup_rnk = WmZCurveLayer2D::off_right<NCellRank + 1>(1, NCnt);
        [[maybe_unused]]
        int64_t x_dup_cnt = WmZCurveLayer2D::off_right<NCellRank>(1, NCnt * 2);
        WM_ASSERT(x_dup_rnk == x_dup_cnt, "TEST FAILED");

        [[maybe_unused]]
        int64_t y_dup_rnk = WmZCurveLayer2D::off_bottom<NCellRank + 1>(1, NCnt);
        [[maybe_unused]]
        int64_t y_dup_cnt = WmZCurveLayer2D::off_bottom<NCellRank>(1, NCnt * 2);
        WM_ASSERT(y_dup_rnk == y_dup_cnt, "TEST FAILED");

        stream << "END WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmZCurveLayer2D::NDomainLength * WmZCurveLayer2D::NDomainLength;

        stream << "BEGIN WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_init()\n";

        WmZCurveLayer2D layer;

        size_t call_cnt = 0;
        TestInitFunc<TStream> init_func(stream, &call_cnt);

        layer.init(1.0, init_func);

        stream << "CALLED " << call_cnt << "\n";
        stream << "SQUARE " << NSquare << "\n";

        WM_ASSERT(call_cnt == NSquare, "TEST FAILED");

        stream << "END WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_init()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_operator(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmZCurveLayer2D::NDomainLength * WmZCurveLayer2D::NDomainLength;

        stream << "BEGIN WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_operator()\n";

        WmZCurveLayer2D layer;
        static_cast<void>(layer[0]);
        static_cast<void>(layer[NSquare - 1]);

        stream << "END WmZCurveLayer2D<$TData, " << 
            WmZCurveLayer2D::NDomainLength << 
            ">::test_operator()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_LAYER_ZCURVE_LAYER_H_
