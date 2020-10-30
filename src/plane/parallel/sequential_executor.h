#ifndef PARALLEL_SEQUENTIAL_EXECUTOR_H_
#define PARALLEL_SEQUENTIAL_EXECUTOR_H_

#include "abstract_executor.h"

#include <functional>

namespace wave_model {

class WmSequentialExecutor final : public WmAbstractExecutor
{
public:
    struct Test;

    // TODO: to check how much threads to run
    WmSequentialExecutor() = default;
    ~WmSequentialExecutor() override final = default;

    void enqueue(std::function<void()> func) override final
    {
        std::move(func)();
    }
};

struct WmSequentialExecutor::Test
{
    template<typename TStream>
    static TStream& test_init(TStream& stream)
    {
        WmSequentialExecutor seq_exec;

        return stream;
    }

    template<typename TStream>
    static TStream& test_run(TStream& stream)
    {
        size_t counter = 0;

        {
            WmSequentialExecutor seq_exec;

            for (size_t idx = 0; idx < 4; ++idx)
            {
                seq_exec.enqueue([&counter, inc = idx + 1]() { 
                        counter += inc; 
                    });
            }
        }

        stream << counter;

        return stream;
    }
};

} // namespace wave_model 

#endif // PARALLEL_SEQUENTIAL_EXECUTOR_H_
