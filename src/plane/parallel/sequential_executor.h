#ifndef PARALLEL_SEQUENTIAL_EXECUTOR_H_
#define PARALLEL_SEQUENTIAL_EXECUTOR_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include "abstract_executor.h"

#include <functional>

/// @brief
namespace wave_model {

/**
 * @brief Executor implementing sequential strategy
 * Is implemented for testing algorithms executed as the simplest one.
 */
class WmSequentialExecutor final : public WmAbstractExecutor
{
public:
    struct Test;

    /**
     * @brief Default Ctor
     */
    WmSequentialExecutor() = default;
    ~WmSequentialExecutor() override final = default;

    /**
     * @brief Immediately executes the task and bloks until executed
     * @param func Task to be executed
     * @see WmAbstractExecutor::enqueue()
     */
    void enqueue(std::function<void()> func) override final
    {
        std::move(func)();
    }
};

/**
 * @brief Test structure for WmSequentialExecutor
 */
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
