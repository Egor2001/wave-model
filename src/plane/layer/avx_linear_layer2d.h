#ifndef WAVE_MODEL_LAYER_AVX_LINEAR_LAYER2D_H_
#define WAVE_MODEL_LAYER_AVX_LINEAR_LAYER2D_H_

#include "logging/macro.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

#include <immintrin.h>

namespace wave_model {

// TODO: border & PML
template<class TD, size_t ND>
class WmAvxLinearLayer2D
{
public:
    struct Test;

    using TData = TD;
    static constexpr int64_t NDomainLength = ND;
    static constexpr int64_t NAvxLength = 4u;

    static_assert(NDomainLength && !(NDomainLength & (NDomainLength - 1)), 
                  "NDomainLength must be 2's power");

    static_assert(NDomainLength > NAvxLength, 
                  "NDomainLength must not be less than avx register");

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_top([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_bottom<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_bottom([[maybe_unused]] uint64_t idx, uint64_t cnt) noexcept
    {
        return ((NDomainLength / NAvxLength) * 
                static_cast<int64_t>((1 << NCellRank) * cnt));
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

    WmAvxLinearLayer2D():
        data_vec_(NDomainLength * (NDomainLength / NAvxLength))
    {}

    template<typename FInitFunc>
    void init(double length, FInitFunc func)
    {
        double scale_factor = length / NDomainLength;

        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        for (int64_t x_idx = 0; x_idx < NDomainLength / NAvxLength; ++x_idx)
        {
            double x = scale_factor * 
                static_cast<double>(x_idx * NAvxLength - NDomainLength / 2);
            double y = scale_factor * 
                static_cast<double>(y_idx - NDomainLength / 2);

            int64_t idx = y_idx * NDomainLength + x_idx;
            data_vec_[idx] = 
                _mm256_set_pd(
                    func(x + 0.0 * scale_factor, y), 
                    func(x + 1.0 * scale_factor, y), 
                    func(x + 2.0 * scale_factor, y), 
                    func(x + 3.0 * scale_factor, y)
                    );
        }
    }

    template<typename TStream>
    TStream& dump(TStream& stream) const noexcept
    {
        for (int64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        {
            for (int64_t x_idx = 0; x_idx < NDomainLength / NAvxLength; ++x_idx)
                stream << data_vec_[y_idx * NDomainLength / NAvxLength + x_idx] 
                       << ' ';

            stream << '\n';
        }

        return stream;
    }

    WmAvxLinearLayer2D(const WmAvxLinearLayer2D&) = delete;
    WmAvxLinearLayer2D& operator = (const WmAvxLinearLayer2D&) = delete;

    WmAvxLinearLayer2D(WmAvxLinearLayer2D&&) noexcept = default;
    WmAvxLinearLayer2D& operator = (WmAvxLinearLayer2D&&) noexcept = default;

    [[nodiscard]] inline TData& operator [] (int64_t idx) noexcept
    {
        WM_ASSERT(0 <= idx && 
                  idx < NDomainLength * (NDomainLength / NAvxLength), 
                  "idx is out of bounds");

        return data_vec_[idx];
    }

    [[nodiscard]] inline const TData& operator [] (int64_t idx) const noexcept
    {
        return const_cast<WmAvxLinearLayer2D*>(this)->operator[](idx);
    }

private:
    std::vector<TData> data_vec_;
};

// TODO: to create templatized testing class for any layer type
// to avoid copy-paste
template<typename TD, size_t ND>
struct WmAvxLinearLayer2D<TD, ND>::Test
{
    template<typename TStream>
    struct TestInitFunc
    {
        explicit TestInitFunc(TStream& stream_ref, size_t* call_cnt_ptr):
            stream(stream_ref),
            call_cnt(call_cnt_ptr)
        {}

        [[nodiscard]]
        WmAvxLinearLayer2D::TData operator () (double x, double y) noexcept
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
        stream << "BEGIN WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        WM_ASSERT(WmAvxLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                (1u << NCellRank) * WmAvxLinearLayer2D::NDomainLength, 
                "TEST FAILED");
        WM_ASSERT(WmAvxLinearLayer2D::off_right<NCellRank>(0, 1) == 
                (1u << NCellRank), "TEST FAILED");

        WM_ASSERT(WmAvxLinearLayer2D::off_bottom<NCellRank>(0, 1) == 
                -WmAvxLinearLayer2D::off_top<NCellRank>(0, 1), "TEST FAILED");
        WM_ASSERT(WmAvxLinearLayer2D::off_right<NCellRank>(0, 1) == 
                -WmAvxLinearLayer2D::off_left<NCellRank>(0, 1), "TEST FAILED");

        WM_ASSERT(WmAvxLinearLayer2D::off_bottom<NCellRank>(0, 1) * 2 == 
                WmAvxLinearLayer2D::off_bottom<NCellRank>(0, 2), "TEST FAILED");
        WM_ASSERT(WmAvxLinearLayer2D::off_right<NCellRank>(0, 1) * 2 == 
                WmAvxLinearLayer2D::off_right<NCellRank>(0, 2), "TEST FAILED");

        stream << "END WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_off<" << NCellRank << ">()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmAvxLinearLayer2D::NDomainLength * 
            WmAvxLinearLayer2D::NDomainLength;

        stream << "BEGIN WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_init()\n";

        WmAvxLinearLayer2D layer;

        size_t call_cnt = 0;
        TestInitFunc<TStream> init_func(stream, &call_cnt);

        layer.init(1.0, init_func);

        stream << "CALLED " << call_cnt << "\n";
        stream << "SQUARE " << NSquare << "\n";

        WM_ASSERT(call_cnt == NSquare, "TEST FAILED");

        stream << "END WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_init()\n";

        return stream;
    }

    template<typename TStream>
    static TStream& test_operator(TStream& stream)
    {
        static constexpr size_t NSquare = 
            WmAvxLinearLayer2D::NDomainLength * 
            WmAvxLinearLayer2D::NDomainLength;

        stream << "BEGIN WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_operator()\n";

        WmAvxLinearLayer2D layer;
        static_cast<void>(layer[0]);
        static_cast<void>(layer[NSquare - 1]);

        stream << "END WmAvxLinearLayer2D<$TData, " << 
            WmAvxLinearLayer2D::NDomainLength << 
            ">::test_operator()\n";

        return stream;
    }
};

} // namespace wave_model

#endif // WAVE_MODEL_LAYER_AVX_LINEAR_LAYER2D_H_
