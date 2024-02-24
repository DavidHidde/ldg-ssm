#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <algorithm>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/tree_functions.hpp"
#include "app/include/shared/util/image.hpp"
#include "targets.hpp"
#include "partitions.hpp"

namespace ssm
{
    /**
     * Get the start height of the SSM algorithm, which starts when we have at least 4 partitions in each dimension.
     *
     * @tparam VectorType
     * @param quad_tree
     * @return
     */
    template<typename VectorType>
    size_t getStartHeight(shared::QuadAssignmentTree<VectorType> &quad_tree)
    {
        using namespace shared;
        size_t height = quad_tree.getDepth() - 1;
        auto dims = quad_tree.getBounds(CellPosition{ height, 0 }).second;
        while (height > 0 && (dims.first < 4 || dims.second < 4)) {
            --height;
            dims = quad_tree.getBounds(CellPosition{ height, 0 }).second;
        }
        return height;
    }

    /**
     * The self-sorting map as detailed in https://doi.org/10.1109/TMM.2014.2306183,
     * specifically for a QuadAssignmentTree given a distance function.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param target_type
     */
    template<typename VectorType>
    void sort(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        const size_t max_iterations,
        const TargetType target_type
    )
    {
        using namespace shared;
        size_t height = getStartHeight(quad_tree);

        // Main loop - We use the height as an indicator of the partition size rather than calculating the partition size.
        size_t iterations;
        size_t num_exchanges;
        for (; height > 0; --height) {
            iterations = 0;

            do {
                num_exchanges = 0;
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_type, height, 0, false);
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_type, height, 0, true);
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_type, height + 1, height, false);
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_type, height + 1, height, true);
                ++iterations;
            } while (iterations < max_iterations && num_exchanges > 0);

            saveQuadTreeImages(quad_tree, "ssm-size(" + std::to_string(size_t(std::pow(2., height))) + ")");
            size_t new_score = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);
            std::cout << "Finished height " << height << " in " << iterations << " iterations with score " << new_score << '\n';
        }

        // Fix aggregates in the end if we did perform swaps in the end.
        if (num_exchanges > 0)
            computeAggregates(quad_tree);
    }
}

#endif //LDG_CORE_METHOD_HPP
