#ifndef GRID_GRAPH_H_
#define GRID_GRAPH_H_

/**
 * @file
 * @author Egor Elchinov <elchinov.es@gmail.com>
 * @version 2.0
 */

#include <vector>
#include <cstdint>

/// @brief
namespace wave_model {

/**
 * @brief Describes structure of some grid
 * Provides dependency graph and sequential traverse order.
 */
struct WmGridGraph
{
    size_t count; ///< vertex count
    std::vector<size_t> order; ///< possible sequential traverse order
    std::vector<std::vector<size_t>> graph; ///< dependency graph
};

} // namespace wave_model

#endif // GRID_GRAPH_H_
