#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <iostream>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/ldg/util/tree_functions.hpp"
#include "targets.hpp"
#include "partitions.hpp"
#include "app/include/program/logger.hpp"
#include "app/include/program/export/export.hpp"

namespace ssm
{
    /**
     * Check if the distance has changed enough within a certain threshold.
     * If the threshold is 0 or lower, we ignore this computation.
     *
     * @param old_distance
     * @param new_distance
     * @param threshold
     * @return
     */
    inline bool distanceHasChanged(double old_distance, double new_distance, double threshold)
    {
        return threshold <= 0 || std::abs(old_distance - new_distance) / old_distance > threshold;
    }

    /**
     * Get the start height of the SSM algorithm, which starts when we have at least 4 partitions in each dimension.
     *
     * @tparam VectorType
     * @param quad_tree
     * @return
     */
    template<typename VectorType>
    size_t getSSMStartHeight(ldg::QuadAssignmentTree<VectorType> &quad_tree)
    {
        using namespace ldg;
        size_t height = quad_tree.getDepth() - 1;
        auto dims = quad_tree.getBounds(height).second;
        while (height > 0 && (dims.first < 4 || dims.second < 4)) {
            --height;
            dims = quad_tree.getBounds(height).second;
        }
        return height;
    }

    /**
     * The self-sorting map (as detailed in https://doi.org/10.1109/TMM.2014.2306183) with some extra parameters and modifications,
     * specifically for a QuadAssignmentTree given a distance function.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param iterations_between_checkpoint
     * @param max_iterations
     * @param distance_threshold
     * @param ssm_mode
     * @param use_partition_swaps
     * @param logger
     * @param export_settings
     */
    template<typename VectorType>
    void sort(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        const size_t iterations_between_checkpoint,
        const size_t max_iterations,
        const double distance_threshold,
        const bool ssm_mode,
        const bool use_partition_swaps,
        program::Logger &logger,
        program::ExportSettings &export_settings
    )
    {
        using namespace ldg;
        double distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);
        double new_distance = distance;

        // Main loop
        long height = ssm_mode ? getSSMStartHeight(quad_tree) : quad_tree.getDepth() - 2;
        size_t num_exchanges;
        std::string reason;
        std::pair<size_t, size_t> normal_pass_pairings{ 2, 2 }; // Separate X-Y passes can be enabled by tweaking this, but this is not needed here.

        for (; height > 0; --height) {
            size_t iterations = 0;

            do {
                num_exchanges = 0;
                num_exchanges += optimizePartitions(quad_tree, distance_function, height, 0, normal_pass_pairings, ssm_mode, false);
                if (height < quad_tree.getDepth() - 2)
                    num_exchanges += optimizePartitions(quad_tree, distance_function, height, 0, normal_pass_pairings, ssm_mode, true);

                if (use_partition_swaps && height > 1) {
                    num_exchanges += optimizePartitions(quad_tree, distance_function, height, height - 1, normal_pass_pairings, ssm_mode, false);
                    num_exchanges += optimizePartitions(quad_tree, distance_function, height, height - 1, normal_pass_pairings, ssm_mode, true);
                }

                distance = new_distance;
                new_distance = computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree);

                if (iterations_between_checkpoint > 0 && iterations > 0 && iterations % iterations_between_checkpoint == 0) {
                    export_settings.file_name = "height-" + std::to_string(height) + "-it(" + std::to_string(iterations) + ')';
                    program::exportQuadTree(quad_tree, distance_function, export_settings);
                }
                logger.write(height, iterations, new_distance, num_exchanges);
                ++iterations;
            } while (iterations < max_iterations && num_exchanges > 0 && distanceHasChanged(distance, new_distance, distance_threshold));

            if (iterations_between_checkpoint > 0) {
                export_settings.file_name = "height-" + std::to_string(height) + "-final";
                program::exportQuadTree(quad_tree, distance_function, export_settings);
            }
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
