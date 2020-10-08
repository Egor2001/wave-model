#ifndef WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_
#define WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_

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
    class Test;

    using TData = TD;
    static constexpr int64_t NDomainLength = ND;

    static_assert(NDomainLength && !(NDomainLength & (NDomainLength - 1)), 
                  "NDomainLength must be 2's power");

    //------------------------------------------------------------ 
    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_top(uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_bottom<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_bottom(uint64_t idx, uint64_t cnt) noexcept
    {
        return (NDomainLength * static_cast<int64_t>((1 << NCellRank) * cnt));
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_left(uint64_t idx, uint64_t cnt) noexcept
    {
        return -off_right<NCellRank>(idx, cnt);
    }

    template<size_t NCellRank> [[nodiscard]] static constexpr 
    int64_t off_right(uint64_t idx, uint64_t cnt) noexcept
    {
        return static_cast<int64_t>((1 << NCellRank) * cnt);
    }
    //------------------------------------------------------------ 

    WmLinearLayer2D():
        data_vec_(NDomainLength * NDomainLength)
    {}

    template<class FInitFunc>
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

    WmLinearLayer2D(const WmLinearLayer2D&) = delete;
    WmLinearLayer2D& operator = (const WmLinearLayer2D&) = delete;

    WmLinearLayer2D(WmLinearLayer2D&&) noexcept = default;
    WmLinearLayer2D& operator = (WmLinearLayer2D&&) noexcept = default;

    TData& operator [] (uint64_t idx) noexcept
    {
        WM_ASSERT(idx < NDomainLength * NDomainLength, 
                  "idx is out of bounds");

        return data_vec_[idx];
    }

    const TData& operator [] (uint64_t idx) const noexcept
    {
        return const_cast<WmLinearLayer2D*>(this)->operator[](idx);
    }

private:
    std::vector<TData> data_vec_;
};

template<class TD, size_t ND>
class WmLinearLayer2D<TD, ND>::Test
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
    TStream& test_get(TStream& stream, int64_t idx)
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
    std::unique_ptr<WmLinearLayer2D> layer_;
};

} // namespace wave_model

#endif // WAVE_MODEL_LAYER_LINEAR_LAYER2D_H_
