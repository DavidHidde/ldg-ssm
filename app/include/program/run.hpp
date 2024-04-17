#ifndef RUN_HPP
#define RUN_HPP

#include "logger.hpp"
#include "schedule.hpp"
#include "sort_options.hpp"
#include "app/include/ldg/util/tree_functions.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/metric/hierarchy_neighborhood_distance.hpp"
#include "app/include/self_sorting_map/method.hpp"

#include <iostream>
#include <bits/fs_path.h>

namespace program
{
    /**
     * Create a schedule for which targets to use at which step. Note that if we don't combine the targets and we have less targets
     * specified than needed, we just repeat the last one.
     *
     * @tparam VectorType
     * @param schedule
     * @param sort_options
     * @return
     */
    template<typename VectorType>
    std::vector<std::vector<ssm::TargetType>> createTargetSchedule(Schedule &schedule, SortOptions<VectorType> &sort_options)
    {
        if (schedule.combine_targets) {
            return std::vector<std::vector<ssm::TargetType>>(schedule.number_of_passes, sort_options.target_types);
        }

        std::vector<std::vector<ssm::TargetType>> target_schedule;
        for (size_t idx = 0; idx < schedule.number_of_passes; ++idx) {
            target_schedule.push_back({sort_options.target_types[std::min(idx, sort_options.target_types.size() - 1)]});
        }
        return target_schedule;
    }

    /**
     * Run the actual sorting procedure on a quad tree following a schedule given set options.
     * Note that we assume everything has been initialized from the input at this point, and we only do a couple of sanity checks before proceeding.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param schedule
     * @param sort_options
     * @param export_settings
     */
    template<typename VectorType>
    void run(ldg::QuadAssignmentTree<VectorType> &quad_tree, Schedule &schedule, SortOptions<VectorType> &sort_options, ExportSettings &export_settings)
    {
        ldg::assertUniqueAssignment(quad_tree);
        std::cout << "Initial HND: " << ldg::computeHierarchyNeighborhoodDistance(0, sort_options.distance_function, quad_tree) << std::endl;
        if (sort_options.randomize_assignment) {
            ldg::randomizeAssignment(quad_tree, sort_options.randomization_seed);
            std::cout << "Randomized HND: " << ldg::computeHierarchyNeighborhoodDistance(0, sort_options.distance_function, quad_tree) << std::endl;
        }
        std::cout << std::endl;

        size_t max_iterations = schedule.max_iterations;
        double distance_threshold = schedule.distance_threshold;
        auto target_schedule = createTargetSchedule(schedule, sort_options);

        // Main loop where we perform the sorting.
        const double start = omp_get_wtime();
        std::string base_output_dir = export_settings.output_dir;
        std::filesystem::create_directories(base_output_dir);
        Logger logger(start, base_output_dir);
        logger.setUsingPartitionSwaps(sort_options.use_partition_swaps).setNumRows(quad_tree.getNumRows()).setNumCols(quad_tree.getNumCols());
        for (size_t idx = 0; idx < schedule.number_of_passes; ++idx) {
            std::cout << "--- Pass " << idx + 1 << " ---" << std::endl;
            logger.setNumPass(idx).setMaxIterations(max_iterations).setDistanceThreshold(distance_threshold).setTargets(target_schedule[idx]);
            std::string pass_output_dir = base_output_dir + "pass" + std::to_string(idx + 1) + std::filesystem::__cxx11::path::preferred_separator;
            std::filesystem::create_directories(pass_output_dir);
            export_settings.output_dir = pass_output_dir;

            ssm::sort(
                quad_tree,
                sort_options.distance_function,
                schedule.iterations_per_checkpoint,
                max_iterations,
                distance_threshold,
                target_schedule[idx],
                sort_options.use_partition_swaps,
                logger,
                export_settings
            );

            distance_threshold *= schedule.threshold_change_factor;
            max_iterations = std::ceil(static_cast<double>(max_iterations) * schedule.iterations_change_factor);
            std::cout << std::endl;
        }

        ldg::assertUniqueAssignment(quad_tree);
        logger.close();
        std::cout << "Final HND: " << ldg::computeHierarchyNeighborhoodDistance(0, sort_options.distance_function, quad_tree) << std::endl;
        printf("Time elapsed: %.5f\n\n", omp_get_wtime() - start);
    }
}

#endif //RUN_HPP