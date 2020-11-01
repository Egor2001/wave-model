#ifndef GRID_GRAPH_H_
#define GRID_GRAPH_H_

#include <vector>
#include <cstdint>

namespace wave_model {

struct WmGridGraph
{
    size_t count;
    std::vector<size_t> order;
    std::vector<std::vector<size_t>> graph;
};

} // namespace wave_model

#endif // GRID_GRAPH_H_
