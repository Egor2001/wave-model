#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_

#include "abstract_executor.h"

namespace wave_model {

class WmAbstractGrid /* abstract */
{
public:
    WmAbstractGrid() = default;

    virtual ~WmAbstractExecutor() = default;
    virtual void traverse(WmAbstractExecutor& executor) = 0;

protected:
    WmAbstractGrid             (const WmAbstractGrid&) = delete;
    WmAbstractGrid& operator = (const WmAbstractGrid&) = delete;

    WmAbstractGrid             (WmAbstractGrid&&) noexcept = default;
    WmAbstractGrid& operator = (WmAbstractGrid&&) noexcept = default;
};

} // namespace wave_model {

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
