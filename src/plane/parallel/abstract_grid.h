#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_

#include "grid_graph.h"

namespace wave_model {

class WmAbstractGrid /* abstract */
{
public:
    WmAbstractGrid() = default;

    virtual ~WmAbstractGrid() = default;
    virtual WmAbstractNode* access_node(size_t idx) = 0;
    virtual WmGridGraph build_graph() const = 0;

protected:
    WmAbstractGrid             (const WmAbstractGrid&) = delete;
    WmAbstractGrid& operator = (const WmAbstractGrid&) = delete;

    WmAbstractGrid             (WmAbstractGrid&&) noexcept = default;
    WmAbstractGrid& operator = (WmAbstractGrid&&) noexcept = default;
};

} // namespace wave_model {

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
