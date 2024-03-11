#ifndef LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
#define LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP

#include <functional>
#include <map>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"

namespace ldg
{
    /**
     * Compute the HND for a quad tree at a given height.
     *
     * @tparam VectorType
     * @param height
     * @param distance_function
     * @param quad_tree
     * @return
     */
    template<typename VectorType>
    double computeHierarchyNeighborhoodDistance(
        size_t height,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        QuadAssignmentTree<VectorType> &quad_tree
    )
    {
        computeAggregates(quad_tree);
        double sum = 0.;
        const size_t height_to_root = quad_tree.getDepth() - height;
        auto [height_bounds, height_dims] = quad_tree.getBounds(CellPosition{ height, 0 });
        const size_t num_elems = height_dims.first * height_dims.second;
        std::vector height_distances(num_elems * height_to_root, 0.);
        std::map<size_t, double> cache;

        // Add all scores together (4-connectivity filter)
#pragma omp parallel for reduction(+:sum) private(cache)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            CellPosition position{ height, idx };
            size_t x = position.index % height_bounds.second;
            size_t y = position.index / height_bounds.second;

            auto node_value = quad_tree.getValue(position);
            double neighbor_sum =
                computeHierarchyDistanceForCell(
                    CellPosition{ height, rowMajorIndex(y, x - 1, height_bounds.second) },
                    node_value,
                    distance_function,
                    quad_tree,
                    cache
                ) +
                computeHierarchyDistanceForCell(
                    CellPosition{ height, rowMajorIndex(y, x + 1, height_bounds.second) },
                    node_value,
                    distance_function,
                    quad_tree,
                    cache
                ) +
                computeHierarchyDistanceForCell(
                    CellPosition{ height, rowMajorIndex(y - 1, x, height_bounds.second) },
                    node_value,
                    distance_function,
                    quad_tree,
                    cache
                ) +
                computeHierarchyDistanceForCell(
                    CellPosition{ height, rowMajorIndex(y + 1, x, height_bounds.second) },
                    node_value,
                    distance_function,
                    quad_tree,
                    cache
                );

            sum += computeHierarchyDistanceForCell(position, node_value, distance_function, quad_tree, cache) + neighbor_sum / 4.;
            cache.clear();
        }

        return sum;
    }

    /**
     * Compute the distance to the upper hierarchy members at a certain position.
     *
     * @tparam VectorType
     * @param position
     * @param value
     * @param distance_function
     * @param quad_tree
     * @param cache Cache of already computed distances between value and parents.
     * @return
     */
    template<typename VectorType>
    double computeHierarchyDistanceForCell(
        CellPosition position,
        std::shared_ptr<VectorType> &value,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        QuadAssignmentTree<VectorType> &quad_tree,
        std::map<size_t, double> &cache
    )
    {
        TreeWalker<VectorType> walker(position, quad_tree);
        walker.moveUp();    // Move to the first parent
        double sum = 0;
        auto [num_rows, num_cols] = quad_tree.getBounds(position).second;

        if (value != nullptr && position.index < num_rows * num_cols) { // Index is size_t, so negative will overflow to max value
            do {
                auto walker_position = walker.getNode();
                size_t key = rowMajorIndex(walker_position.height, walker_position.index, quad_tree.getNumCols());
                if (!cache.count(key)) {    // We haven't computed this yet
                    cache[key] = distance_function(value, walker.getNodeValue());
                }
                sum += cache[key];
            } while (walker.moveUp());
        }

        return sum;
    }
}

#endif //LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
