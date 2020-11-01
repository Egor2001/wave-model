#ifndef WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
#define WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include "grid_graph.h"

/// @brief
namespace wave_model {

/**
 * @brief Abstact class describing grid
 * The grid consists of the nodes each executing on single thread/core/unit.
 * It just holds its nodes and provides interface to get each node and 
 * the grid configuration (grid graph).
 */
class WmAbstractGrid /* abstract */
{
public:
    WmAbstractGrid() = default;

    virtual ~WmAbstractGrid() = default;

    /**
     * @brief Returns pointer to idx'th node in internal order.
     * @param idx Node's index
     * @return Node's pointer
     */
    virtual WmAbstractNode* access_node(size_t idx) = 0;

    /**
     * @brief Builds and returns grid configuration as a graph
     * Vertices in graph are listed in the internal order 
     * same as for the access_node() function.
     * @returns Corresponding graph
     */
    virtual WmGridGraph build_graph() const = 0;

protected:
    WmAbstractGrid             (const WmAbstractGrid&) = delete;
    WmAbstractGrid& operator = (const WmAbstractGrid&) = delete;

    WmAbstractGrid             (WmAbstractGrid&&) noexcept = default;
    WmAbstractGrid& operator = (WmAbstractGrid&&) noexcept = default;
};

} // namespace wave_model {

#endif // WAVE_MODEL_PARALLEL_ABSTRACT_GRID_H_
