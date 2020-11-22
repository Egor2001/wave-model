#ifndef WAVE_MODEL_LOGGING_BENCH_H_
#define WAVE_MODEL_LOGGING_BENCH_H_

#include <chrono>
#include <string_view>

namespace wave_model {

template<typename TStream>
class WmBench
{
public:
    WmBench(TStream& stream, std::string_view name):
        name_{ name },
        stream_{ stream },
        start_tp_{ std::chrono::system_clock::now() }
    {}

    WmBench             (const WmBench&) = delete;
    WmBench& operator = (const WmBench&) = delete;
    WmBench             (WmBench&&) = delete;
    WmBench& operator = (WmBench&&) = delete;

    ~WmBench()
    {
        auto diff = std::chrono::system_clock::now() - start_tp_;
        stream_ << name_ << ' ' << diff.count() << '\n';
    }

private:
    std::string_view name_;
    TStream& stream_;
    std::chrono::time_point<std::chrono::system_clock> start_tp_;
};

} // namespace wave_model

#endif // WAVE_MODEL_LOGGING_BENCH_H_
