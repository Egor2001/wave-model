#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_

#include <functional>

namespace wave_model {

class WmAbstractExecutor /* abstract */
{
public:
    WmAbstractExecutor() = default;

    virtual ~WmAbstractExecutor() = 0;
    virtual void enqueue(std::function<void()>);

protected:
    WmAbstractExecutor& operator = (const WmAbstractExecutor&) = delete;
    WmAbstractExecutor             (const WmAbstractExecutor&) = delete;

    WmAbstractExecutor& operator = (WmAbstractExecutor&&) noexcept = default;
    WmAbstractExecutor             (WmAbstractExecutor&&) noexcept = default;
};

} // namespace wave_model {

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_EXECUTOR_H_
