#ifndef LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
#define LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP

#include <functional>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace shared
{
    const float EMPTY_HND_SCORE = -1.;

    /**
     * Generate the same sized caching tree used for the HND. All values are set to empty initially.
     *
     * @tparam DataType
     * @param source
     * @return
     */
    template<typename DataType>
    QuadAssignmentTree<float> generateCacheTree(QuadAssignmentTree<DataType> &quad_tree)
    {
        auto empty_scores = std::make_shared<std::vector<std::shared_ptr<float>>>(
            quad_tree.getData()->size(),
            std::make_shared<float>(EMPTY_HND_SCORE)
        );
        return QuadAssignmentTree<float>(
            empty_scores,
            quad_tree.getAssignment(),
            quad_tree.getNumRows(),
            quad_tree.getNumCols(),
            quad_tree.getDepth()
        );
    }

    /**
     * Compute the HND for a quad tree at a given height.
     *
     * @tparam DataType
     * @param height
     * @param distance_function
     * @param source
     * @param cache
     * @return
     */
    template<typename DataType>
    float computeHierarchyNeighborhoodDistance(
        size_t height,
        std::function<float(std::shared_ptr<DataType>, std::shared_ptr<DataType>)> distance_function,
        QuadAssignmentTree<DataType> &quad_tree
    )
    {
        computeAggregates(quad_tree);
        float sum = 0;
        auto cache = generateCacheTree(quad_tree);

        // Compute all single scores
        for (RowMajorIterator<float> iterator(height, cache); iterator != iterator.end(); ++iterator) {
            auto value = computeHierarchyDistanceForCell(iterator.getPosition(), distance_function, quad_tree);
            cache.setValue(iterator.getPosition(), value);
        }

        // Add all scores together (4-connectivity filter)
        for (RowMajorIterator<float> iterator(height, cache); iterator != iterator.end(); ++iterator) {
            auto position = iterator.getPosition();
            size_t x = position.index % quad_tree.getNumCols();
            size_t y = position.index / quad_tree.getNumCols();

            float neighbor_sum = 0.;
            size_t neighbour_count = 0;
            if (x > 0) {
                neighbor_sum += *cache.getValue(CellPosition{ 0, position.index - 1 });
                ++neighbour_count;
            }
            if (x < cache.getNumCols() - 1) {
                neighbor_sum += *cache.getValue(CellPosition{ 0, position.index + 1 });
                ++neighbour_count;
            }
            if (y > 0) {
                neighbor_sum += *cache.getValue(CellPosition{ 0, position.index - cache.getNumCols() });
                ++neighbour_count;
            }
            if (y < cache.getNumRows() - 1) {
                neighbor_sum += *cache.getValue(CellPosition{ 0, position.index + cache.getNumCols() });
                ++neighbour_count;
            }

            sum += *iterator.getValue() + neighbor_sum / (neighbour_count > 0 ? float(neighbour_count) : 1.);
        }

        return sum;
    }

    /**
     * Compute the distance to the upper hierarchy members at a certain position.
     *
     * @tparam DataType
     * @param position
     * @param distance_function
     * @param source
     * @return
     */
    template<typename DataType>
    float computeHierarchyDistanceForCell(
        CellPosition &position,
        std::function<float(std::shared_ptr<DataType>, std::shared_ptr<DataType>)> distance_function,
        QuadAssignmentTree<DataType> &source
    )
    {
        TreeWalker<DataType> walker(position, source);
        auto node = walker.getNodeValue();
        float sum = 0;

        if (node != nullptr) {
            do {
                sum += distance_function(node, walker.getParentValue());
            } while (walker.moveUp());
        }

        return sum;
    }
}

#endif //LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
