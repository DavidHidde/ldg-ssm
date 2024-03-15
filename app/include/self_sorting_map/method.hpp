#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <iostream>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/ldg/tree_functions.hpp"
#include "targets.hpp"
#include "partitions.hpp"
#include "app/include/program/logger.hpp"

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
    inline bool distanceHasChanged(double old_distance, double new_distance, double threshold)
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
    size_t getStartHeight(ldg::QuadAssignmentTree<VectorType> &quad_tree)
    {
        using namespace ldg;
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
     * @param checkpoint_function
     * @param iterations_between_checkpoint
     * @param max_iterations
     * @param distance_threshold
     * @param target_types
     * @param output_dir
     * @param use_partition_swaps
     * @param logger
     */
    template<typename VectorType>
    void sort(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::function<void(ldg::QuadAssignmentTree<VectorType> &, std::string)> checkpoint_function,
        const size_t iterations_between_checkpoint,
        const size_t max_iterations,
        const double distance_threshold,
        const std::vector<TargetType> target_types,
        std::string const &output_dir,
        const bool use_partition_swaps,
        program::Logger &logger
    )
    {
        using namespace ldg;
        double distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);
        double new_distance = distance;

        // Main loop
        size_t height = getStartHeight(quad_tree);
        size_t num_exchanges;
        std::string reason;
        for (; height > 0; --height) {
            size_t iterations = 0;

            do {
                num_exchanges = 0;
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, 0, false);
                num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, 0, true);

                if (use_partition_swaps && height > 1) {
                    num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, height - 1, false);
                    num_exchanges += optimizePartitions(quad_tree, distance_function, target_types, height, height - 1, true);
                }

                distance = new_distance;
                new_distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);

                if (iterations_between_checkpoint > 0 && iterations % iterations_between_checkpoint == 0) {
                    checkpoint_function(quad_tree, output_dir + "height-" + std::to_string(height) + "-it(" + std::to_string(iterations) + ')');
                    logger.write(height, iterations, new_distance, num_exchanges);
                }
                ++iterations;
            } while (iterations < max_iterations && num_exchanges > 0 && distanceHasChanged(distance, new_distance, distance_threshold));

            checkpoint_function(quad_tree, output_dir + "height-" + std::to_string(height) + "-final");
            logger.write(height, iterations, new_distance, num_exchanges);
            if (iterations >= max_iterations) {
                reason = " (max iterations reached)";
            } else if (num_exchanges == 0) {
                reason = " (no exchanges left)";
            } else {
                reason = " (distance change below threshold)";
            }
            std::cout << "Finished height " << height << " in " << iterations << " iterations with distance " << new_distance << reason << std::endl;
        }
    }
}

#endif //LDG_CORE_METHOD_HPP
