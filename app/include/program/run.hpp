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
            ldg::randomizeAssignment(quad_tree);
            std::cout << "Randomized HND: " << ldg::computeHierarchyNeighborhoodDistance(0, sort_options.distance_function, quad_tree) << std::endl;
        }
        std::cout << std::endl;

        size_t max_iterations = sort_options.max_iterations;
        double distance_threshold = sort_options.distance_threshold;

        // Main loop where we perform the sorting.
        const double start = omp_get_wtime();
        std::string base_output_dir = export_settings.output_dir;
        std::filesystem::create_directories(base_output_dir);
        Logger logger(start, base_output_dir);
        logger.setNumRows(quad_tree.getNumRows()).setNumCols(quad_tree.getNumCols());
        for (size_t idx = 0; idx < schedule.number_of_passes; ++idx) {
            std::cout << "--- Pass " << idx + 1 << " ---" << std::endl;
            logger.setNumPass(idx).setMaxIterations(max_iterations).setDistanceThreshold(distance_threshold);

            if (schedule.passes_per_checkpoint > 0 && idx % schedule.passes_per_checkpoint == 0) {
                std::string pass_output_dir = base_output_dir + "pass" + std::to_string(idx + 1) + std::filesystem::__cxx11::path::preferred_separator;
                std::filesystem::create_directories(pass_output_dir);
                export_settings.output_dir = pass_output_dir;
            }

            ssm::sort(
                quad_tree,
                sort_options.distance_function,
                schedule.iterations_per_checkpoint,
                max_iterations,
                distance_threshold,
                sort_options.ssm_mode,
                logger,
                export_settings
            );
            std::cout << std::endl;
        }

        ldg::assertUniqueAssignment(quad_tree);
        logger.close();

        export_settings.output_dir = base_output_dir;
        export_settings.file_name = "final";
        program::exportQuadTree(quad_tree, sort_options.distance_function, export_settings);

        std::cout << "Final HND: " << ldg::computeHierarchyNeighborhoodDistance(0, sort_options.distance_function, quad_tree) << std::endl;
        printf("Time elapsed: %.5f\n\n", omp_get_wtime() - start);
    }
}

#endif //RUN_HPP