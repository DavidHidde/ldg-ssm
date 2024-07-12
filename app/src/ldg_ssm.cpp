#include <vector>
#include <memory>
#include "app/include/ldg/util/math.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/adapter/data.hpp"
#include "app/include/ldg/util/metric/distance_function_types.hpp"
#include "app/include/program/run.hpp"
#include "app/include/program/input/input_args.hpp"
#include "app/include/program/input/input.hpp"


/**
 * Entrypoint of the application. Handles input and then delegates to the runner.
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char **argv)
{
    auto parse_result = program::parseInput(argc, argv);

    try {
        if (parse_result.count("cores"))
            omp_set_num_threads(parse_result["cores"].as<size_t>());

        auto [data, assignment, dims, depth, num_elements, data_dims] = program::loadDataFromInput<Eigen::VectorXd>(parse_result);
        auto quad_tree = ldg::QuadAssignmentTree<Eigen::VectorXd>(data, assignment, dims.first, dims.second, depth, num_elements, data_dims, static_cast<ldg::ParentType>(parse_result["parent_type"].as<size_t>()));
        auto schedule = program::loadScheduleFromInput(parse_result);
        auto sort_options = program::loadSortOptionsFromInput<Eigen::VectorXd>(parse_result);
        auto export_settings = program::loadExportSettingsFromInput(parse_result);
        program::run(quad_tree, schedule, sort_options, export_settings);
    } catch (const std::exception &exception) {
        std::cerr << "ldg_ssm: " << exception.what() << std::endl;
        std::cerr << "Something went wrong during excecution. Exiting..." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}