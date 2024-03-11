#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <iostream>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/tree_functions.hpp"
#include "targets.hpp"
#include "partitions.hpp"

namespace ssm
{
    /**
     * Check if the distance has changed enough within a certain threshold.
     *
     * @param old_distance
     * @param new_distance
     * @param threshold
     * @return
     */
    bool distanceHasChanged(double old_distance, double new_distance, double threshold)
    {
        return std::abs(old_distance - new_distance) / old_distance > threshold;
    }

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
     * @param checkpoint_function
     * @param distance_function
     * @param target_types
     */
    template<typename VectorType>
    void sort(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::function<void(shared::QuadAssignmentTree<VectorType> &, std::string const)> checkpoint_function,
        const size_t max_iterations,
        const double distance_threshold,
        const std::vector<TargetType> target_types
    )
    {
        using namespace shared;
        double distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);
        double new_distance = distance;
        std::cout << "Start HND: " << distance << "\n\n";

        // Main loop
        size_t height = getStartHeight(quad_tree);
        size_t iterations;
        size_t num_exchanges;
        std::string reason;
        for (; height > 0; --height) {
            iterations = 0;

            do {
                num_exchanges = 0;
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, 0, false);
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, 0, true);

                if (height > 1) {
                    num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, height - 1, false);
                    num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, height - 1, true);
                }

                distance = new_distance;
                new_distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);
                ++iterations;
            } while (iterations < max_iterations && num_exchanges > 0 && distanceHasChanged(distance, new_distance, distance_threshold));

            checkpoint_function(quad_tree, "height-" + std::to_string(height) + "-final");
            if (iterations >= max_iterations) {
                reason = " (max iterations reached)\n";
            } else if (num_exchanges == 0) {
                reason = " (no exchanges left)\n";
            } else {
                reason = " (distance change below threshold)\n";
            }
            std::cout << "Finished height " << height << " in " << iterations << " iterations with distance " << new_distance << reason;
        }

        std::cout << "\nFinal HND: " << new_distance << '\n';
    }
}

#endif //LDG_CORE_METHOD_HPP
