#include <bits/stdc++.h>

constexpr double LENGTH_EPS = 1e-10;

template<typename FInit>
std::vector<double> init_state(uint32_t points_cnt, FInit&& init_f);

void calc_next_state(std::vector<double>& state_vec,
                     const std::vector<double>& prev_state_vec,
                     double length, double delta_time, double factor2);

template<typename TStream>
TStream& write_to_csv(TStream& stream, const std::vector<double>& state_vec);

void test_naive();

template<typename FInit>
std::vector<double> init_state(uint32_t points_cnt, FInit&& init_f)
{
    std::vector<double> result(points_cnt);
    for (uint32_t point_idx = 0u; point_idx < points_cnt; ++point_idx)
        result[point_idx] = std::forward<FInit>(init_f)(point_idx);

    return result;
}

void calc_next_state(std::vector<double>& state_vec, 
                     const std::vector<double>& prev_state_vec, 
                     double length, double delta_time, double factor2)
{
    const uint32_t points_cnt = state_vec.size();

    // checking for out-of-bound length
    [[unlikely]] if (std::abs(length) < LENGTH_EPS)
        throw std::runtime_error("length < LENGTH_EPS");

    // checking for empty point set
    [[unlikely]] if (points_cnt == 0u)
        throw std::runtime_error("points_cnt == 0u");

    // calc 1 / step
    double inv_step = static_cast<double>(points_cnt) / length;

    // calc courant**2
    double courant2 = 
        factor2 * (delta_time * delta_time) * (inv_step * inv_step);

    // calc next state
    state_vec.front() = state_vec.back() = 0u;
    for (uint32_t point_idx = 1u; point_idx + 1u < points_cnt; ++point_idx)
    {
        state_vec[point_idx] = -state_vec[point_idx] + 
            (prev_state_vec[point_idx + 1u] + 
             prev_state_vec[point_idx - 1u] - 
             2.0 * prev_state_vec[point_idx]) * courant2;
    }
}

template<typename TStream>
TStream& write_to_csv(TStream& stream, const std::vector<double>& state_vec)
{
    for (double displacement : state_vec)
        stream << displacement << ", ";

    stream << '\n';

    return stream;
}

void test_naive()
{
    constexpr double MAX_TIME = 1e3;
    constexpr double DELTA_TIME = 1.0;
    constexpr double LENGTH = 1e5;
    constexpr double FACTOR = 1.0;

    const uint32_t points_cnt = 16u;
    auto fill_f = [points_cnt](uint32_t point_idx) -> double
    { 
        double result = 1.0;
        return (point_idx < points_cnt / 2 ? 
                (result * point_idx) / (points_cnt / 2) :
                (result * (points_cnt - point_idx)) / (points_cnt / 2));
    };

    auto prev_state_vec = init_state(points_cnt, fill_f);
    auto state_vec = prev_state_vec;
    for (double time = 0.0; time < MAX_TIME; time += DELTA_TIME)
    {
        calc_next_state(state_vec, prev_state_vec, LENGTH, DELTA_TIME, FACTOR);
        std::swap(state_vec, prev_state_vec);
    }

    write_to_csv(std::cout, state_vec);
}
