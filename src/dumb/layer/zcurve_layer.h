#ifndef WAVE_MODEL_SOLVER_LAYER_H_
#define WAVE_MODEL_SOLVER_LAYER_H_

#include <vector>
#include <algorithm>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace {

// TODO: border & PML
template<class TD, size_t ND>
class WmZCurveLayer2D
{
public:
    class Test;

    using TData = TD;
    static constexpr int64_t NDomainLength = ND;

    static_assert(NDomainLength && !(NDomainLength & (NDomainLength - 1)), 
                  "NDomainLength must be 2's power");

    // '01010101' x 8
    static constexpr uint64_t ZOrderMask = 0x5555555555555555;
    // Z-order sequence first 8 values 
    static constexpr uint64_t ZOffsetArr[] = { 0, 1, 4, 5, 16, 17, 20, 21 };

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr
    int64_t off_top(uint64_t idx, uint64_t cnt) noexcept
    {
        static constexpr NOffset = (1 << NCellRank) * (1 << NCellRank);
        // static_assert(NCellRank < NQuadRank, "NCellRank is too big");
        // assert(cnt < sizeof(sizeof(ZOffsetArr)/sizeof(*ZOffsetArr)));

        return static_cast<int64_t>(
                (((idx | (ZOrderMask << 1)) - NOffset * ZOffsetArr[cnt]) & 
                (ZOrderMask << 1)) | (idx & ZOrderMask)
                ) - static_cast<int64_t>(idx);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr
    int64_t off_bottom(uint64_t idx, uint64_t cnt) noexcept
    {
        static constexpr NOffset = (1 << NCellRank) * (1 << NCellRank);
        // static_assert(NCellRank < NQuadRank, "NCellRank is too big");
        // assert(cnt < sizeof(sizeof(ZOffsetArr)/sizeof(*ZOffsetArr)));

        return static_cast<int64_t>(
                (((idx & ZOrderMask) + NOffset * ZOffsetArr[cnt]) & 
                (ZOrderMask << 1)) | (idx & ZOrderMask)
                ) - static_cast<int64_t>(idx);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr
    int64_t off_left(uint64_t idx, uint64_t cnt) noexcept
    {
        static constexpr NOffset = (1 << NCellRank) * (1 << NCellRank);
        // static_assert(NCellRank < NQuadRank, "NCellRank is too big");
        // assert(cnt < sizeof(sizeof(ZOffsetArr)/sizeof(*ZOffsetArr)));

        return static_cast<int64_t>(
                (((idx & ZOrderMask) - NOffset * ZOffsetArr[cnt]) & 
                ZOrderMask) | (idx & (ZOrderMask << 1))
                ) - static_cast<int64_t>(idx);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr
    int64_t off_right(uint64_t idx, uint64_t cnt) noexcept
    {
        static constexpr NOffset = (1 << NCellRank) * (1 << NCellRank);
        // static_assert(NCellRank < NQuadRank, "NCellRank is too big");
        // assert(cnt < sizeof(sizeof(ZOffsetArr)/sizeof(*ZOffsetArr)));

        return static_cast<int64_t>(
                (((idx | (ZOrderMask << 1)) + NOffset * ZOffsetArr[cnt]) & 
                ZOrderMask) | (idx & (ZOrderMask << 1))
                ) - static_cast<int64_t>(idx);
    }
    //------------------------------------------------------------ 

    explicit WmZCurveLayer2D(double length):
        length_(length),
        data_vec_(NDomainLength * NDomainLength)
    {}

    template<class FInitFunc>
    WmZCurveLayer2D(double length, FInitFunc func):
        WmZCurveLayer2D(length)
    {
        double scale_factor = length / NDomainLength;

        for (uint64_t y_idx = 0; y_idx < NDomainLength; ++y_idx)
        {
            for (uint64_t x_idx = 0; x_idx < NDomainLength; ++x_idx)
            {
                double x = x_idx - NDomainLength / 2;
                double y = y_idx - NDomainLength / 2;

                data_vec_[idx] = func(x, y);

                idx = off_right<0>(idx, 1);
            }

            idx = off_bottom<0>(idx, 1);
        }
    }

    WmZCurveLayer2D(const WmZCurveLayer2D&) = delete;
    WmZCurveLayer2D& operator = (const WmZCurveLayer2D&) = delete;

    WmZCurveLayer2D(WmZCurveLayer2D&&) noexcept = default;
    WmZCurveLayer2D& operator = (WmZCurveLayer2D&&) noexcept = default;

    // TODO: to implement in terms of single values instead of quads
    TData& operator [] (uint64_t idx)
    {
        return data_vec_[idx];
    }

    const TData& operator [] (uint64_t idx) const
    {
        return const_cast<WmLayer2D*>(this)->operator[](idx);
    }

private:
    double length_;
    std::vector<TData> data_vec_;
};

template<class TD, size_t ND>
class WmZCurveLayer2D<TD, ND>::Test
{
public:
    Test() = default;

    Test             (const Test&) = delete;
    Test& operator = (const Test&) = delete;

    template<typename TStream, typename... Types>
    TStream& test_init(TStream& stream, Types&&... args)
    {
        stream << "WmZCurveLayer2D::Test::test_init()\n";
        try 
        {
            layer_ = std::make_unique<WmLinearLayer2D>
                (std::forward<Types>(args)...);
        }
        catch (std::exception& ex)
        {
            stream << "WmZCurveLayer2D::Test failed: " << ex.what() << "\n";
        }
        catch (...)
        {
            stream << "WmZCurveLayer2D::Test failed: UNKNOWN ERROR\n";
        }

        return stream;
    }

    template<typename TStream>
    TStream& test_release(TStream& stream)
    {
        stream << "WmZCurveLayer2D::Test::test_release()\n";
        layer_.reset();

        return stream;
    }

    template<typename TStream>
    TStream& test_get(int64_t idx)
    {
        stream << "WmZCurveLayer2D::Test::test_get()\n";
        if (layer_)
        {
            try
            {
                [[maybe_unused]]
                auto& data = (*layer_)[idx];
            }
            catch (std::exception& ex)
            {
                stream << "WmZCurveLayer2D::Test failed: " << ex.what() << "\n";
            }
            catch (...)
            {
                stream << "WmZCurveLayer2D::Test failed: UNKNOWN ERROR\n";
            }
        }
        else
        {
            stream << "WmZCurveLayer2D::Test failed: NULL layer_\n";
        }
    }

private:
    std::unique_ptr<WmZCurveLayer2D> layer_;
};

} // namespace

#endif // WAVE_MODEL_SOLVER_LAYER_H_
