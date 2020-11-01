#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include <functional>

/// @brief
namespace wave_model {

/**
 * @brief Abstract class describing task executor
 */
class WmAbstractExecutor /* abstract */
{
public:
    WmAbstractExecutor() = default;

    virtual ~WmAbstractExecutor() = default;

    /**
     * @brief Enqueues new task to execute
     * @param task New task
     */
    virtual void enqueue(std::function<void()> task) = 0;

protected:
    WmAbstractExecutor& operator = (const WmAbstractExecutor&) = delete;
    WmAbstractExecutor             (const WmAbstractExecutor&) = delete;

    WmAbstractExecutor& operator = (WmAbstractExecutor&&) noexcept = default;
    WmAbstractExecutor             (WmAbstractExecutor&&) noexcept = default;
};

} // namespace wave_model {

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_
