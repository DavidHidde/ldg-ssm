#ifndef LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
#define LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP

#include <functional>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace shared
{
    const double EMPTY_HND_SCORE = -1.;

    /**
     * Generate the same sized caching tree used for the HND. All values are set to empty initially.
     *
     * @tparam VectorType
     * @param source
     * @return
     */
    template<typename VectorType>
    QuadAssignmentTree<double> generateCacheTree(QuadAssignmentTree<VectorType> &quad_tree)
    {
        std::vector<std::shared_ptr<double>> empty_scores(quad_tree.getData().size());
#pragma omp parallel for
        for (size_t idx = 0; idx < quad_tree.getNumRows() * quad_tree.getNumCols(); ++idx)
            empty_scores[idx] = std::make_shared<double>(EMPTY_HND_SCORE);  // Explicitly create a new pointer for each element

        return QuadAssignmentTree<double>(
            empty_scores,
            quad_tree.getAssignment(),
            quad_tree.getNumRows(),
            quad_tree.getNumCols(),
            quad_tree.getDepth(),
            quad_tree.getNumRealElements(),
            quad_tree.getDataElementLen()
        );
    }

    /**
     * Compute the HND for a quad tree at a given height.
     *
     * @tparam VectorType
     * @param height
     * @param distance_function
     * @param source
     * @param cache
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
        auto cache = generateCacheTree(quad_tree);

        // Compute all single scores
        auto height_bounds = quad_tree.getBounds(CellPosition{ height, 0 });
        auto height_array_bounds = height_bounds.first;
        auto height_array_dims = height_bounds.second;
#pragma omp parallel for
        for (size_t idx = height_array_bounds.first; idx < height_array_bounds.second; ++idx)  {
            CellPosition position{ height, idx };
            auto value = computeHierarchyDistanceForCell(position, distance_function, quad_tree);
            cache.setValue(position, value);
        }

        // Add all scores together (4-connectivity filter)
#pragma omp parallel for reduction(+:sum)
        for (size_t idx = height_array_bounds.first; idx < height_array_bounds.second; ++idx)  {
            CellPosition position{ height, idx };
            size_t x = position.index % quad_tree.getNumCols();
            size_t y = position.index / quad_tree.getNumCols();

            double neighbor_sum = 0.;
            size_t neighbour_count = 0;
            if (x > 0) {
                neighbor_sum += *cache.getValue(CellPosition{ height, position.index - 1 });
                ++neighbour_count;
            }
            if (x < height_array_dims.second - 1) {
                neighbor_sum += *cache.getValue(CellPosition{ height, position.index + 1 });
                ++neighbour_count;
            }
            if (y > 0) {
                neighbor_sum += *cache.getValue(CellPosition{ height, position.index - height_array_dims.second });
                ++neighbour_count;
            }
            if (y < height_array_dims.first - 1) {
                neighbor_sum += *cache.getValue(CellPosition{ height, position.index + height_array_dims.second });
                ++neighbour_count;
            }

            sum += *cache.getValue(position) + neighbor_sum / (neighbour_count > 0 ? double(neighbour_count) : 1.);
        }

        return sum;
    }

    /**
     * Compute the distance to the upper hierarchy members at a certain position.
     *
     * @tparam VectorType
     * @param position
     * @param distance_function
     * @param source
     * @return
     */
    template<typename VectorType>
    double computeHierarchyDistanceForCell(
        CellPosition &position,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        QuadAssignmentTree<VectorType> &source
    )
    {
        TreeWalker<VectorType> walker(position, source);
        auto node = walker.getNodeValue();
        double sum = 0;

        if (node != nullptr) {
            do {
                sum += distance_function(node, walker.getParentValue());
            } while (walker.moveUp());
        }

        return sum;
    }
}

#endif //LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
