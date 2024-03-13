#include <vector>
#include <memory>
#include <cxxopts.hpp>
#include <Eigen/Dense>
#include "app/include/ldg/util/math.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/ldg/util/image.hpp"
#include "app/include/ldg/util/metric/hierarchy_neighborhood_distance.hpp"
#include "app/include/adapter/data.hpp"
#include "app/include/adapter/storage.hpp"
#include "app/include/ldg/util/metric/cosine_distance.hpp"
#include "app/include/ldg/util/metric/distance_function_types.hpp"
#include "app/include/ldg/util/metric/euclidean_distance.hpp"
#include "app/include/program/run.hpp"
#include "app/include/program/schedule.hpp"
#include "app/include/program/sort_options.hpp"

/**
 * Generate some random RGB data in the range [0, 255], already in the quad tree structure.
 *
 * @tparam VectorType
 * @param num_rows
 * @param num_cols
 * @return
 */
template<typename VectorType>
std::vector<std::shared_ptr<VectorType>> generateUniformRGBData(size_t num_rows, size_t num_cols)
{
    // Generate quad tree structure space, initialized to an empty shared ptr
    size_t num_elements = num_rows * num_cols;
    size_t size = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
    auto data = std::vector<std::shared_ptr<Eigen::VectorXd>>(size, std::make_shared<Eigen::VectorXd>(Eigen::Vector3d::Zero(3)));

    // Fill first cells with data
    for (size_t idx = 0; idx < size; ++idx) {
        if (idx < num_elements) {
            size_t x = idx % num_cols;
            size_t y = idx / num_cols;
            double r = x * (255. / (num_cols - 1));
            double g = (x + y) * (255. / (num_cols + num_rows - 2));
            double b = y * (255. / (num_rows - 1));
            data[idx] = std::make_shared<VectorType>(VectorType{{std::round(r), std::round(g), std::round(b)}});
        } else {
            data[idx] = std::make_shared<VectorType>(VectorType::Zero(3));
        }
    }

    return data;
}

/**
 * Load the quad tree data from the input arguments.
 * Exits if arguments are invalid or missing.
 *
 * @tparam VectorType
 * @param result
 * @return
 */
template<typename VectorType>
std::tuple<std::vector<std::shared_ptr<VectorType>>, std::vector<size_t>, std::pair<size_t, size_t>, size_t, size_t, size_t> loadDataFromInput(cxxopts::ParseResult const &result)
{
    size_t num_rows = result["rows"].as<size_t>();
    size_t num_cols = result["columns"].as<size_t>();

    // Debug mode: generate uniform synthetic RGB data
    if (result["debug"].as<bool>()) {
        num_rows = num_rows == 0 ? 128 : num_rows;// Default to 128
        num_cols = num_cols == 0 ? 128 : num_cols;// Default to 128
        auto data = generateUniformRGBData<VectorType>(num_rows, num_cols);
        auto assignment = ldg::createAssignment(data.size());
        return {
            data,
            assignment,
            {num_rows, num_cols},
            std::ceil(std::log2(std::max(num_rows, num_cols))) + 1,
            num_rows * num_cols,
            3
        };
    }

    if (result.count("config") != 1) {
        std::cerr << "Missing config for non-debug mode operation. Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    auto [data, dims, element_len, num_elements] = adapter::loadData<VectorType>(result["config"].as<std::string>(), num_rows, num_cols);
    num_rows = dims.first;
    num_cols = dims.second;
    std::vector<size_t> assignment = result.count("input")
        ? adapter::readCompressedAssignment(
            result["input"].as<std::string>(),
            num_rows,
            num_cols,
            num_elements
        ) : ldg::createAssignment(data.size());

    return {
        data,
        assignment,
        dims,
        std::ceil(std::log2(std::max(num_rows, num_cols))) + 1,
        num_elements,
        element_len
    };
}

/**
 * Load the schedule data from the input arguments.
 * Exits if arguments are invalid or missing.
 *
 * @param result
 * @return
 */
program::Schedule loadScheduleFromInput(cxxopts::ParseResult const &result)
{
    return {
        result["passes"].as<size_t>(),
        result["max_iterations"].as<size_t>(),
        result["iterations_change_factor"].as<double>(),
        result["min_distance_change"].as<double>(),
        result["distance_change_factor"].as<double>(),
        result["iterations_per_checkpoint"].as<size_t>(),
        result["combine_targets"].as<bool>(),
    };
}

/**
 * Load the sort options data from the input arguments.
 * Exits if arguments are invalid or missing.
 *
 * @tparam VectorType
 * @param result
 * @return
 */
template<typename VectorType>
program::SortOptions<VectorType> loadSortOptionsFromInput(cxxopts::ParseResult const &result)
{
    auto parsed_targets = result["targets"].as<std::vector<size_t>>();
    std::vector<ssm::TargetType> targets(parsed_targets.size());
    std::transform(parsed_targets.begin(), parsed_targets.end(), targets.begin(), [](size_t x) { return static_cast<ssm::TargetType>(x);});

    return {
        result["randomize"].as<bool>(),
        result["seed"].as<size_t>(),
        result["partition_swaps"].as<bool>(),
        targets,
        ldg::mapFunctionTypeToFunction<VectorType>(static_cast<ldg::DistanceFunctionType>(result["distance_function"].as<size_t>())),
        result["debug"].as<bool>() ? ldg::saveQuadTreeRGBImages<VectorType> : adapter::saveAndCompressAssignment<VectorType>
    };
}

/**
 * Entrypoint of the application. Handles input and then delegates to the runner.
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char **argv)
{
    cxxopts::Options options("new_ldg");
    options.add_options()
        ("config", "Path to the config file.", cxxopts::value<std::string>())
        ("input", "Path to the previous assignment file.", cxxopts::value<std::string>())
        ("output", "Path to the output directory.", cxxopts::value<std::string>()->default_value("./"))
        ("cores", "Number of cores to use for parallel operations.", cxxopts::value<size_t>())
        ("passes", "Number of passes.", cxxopts::value<size_t>()->default_value("1"))
        ("max_iterations", "Number of maximum iterations for convergence.", cxxopts::value<size_t>()->default_value("1000"))
        ("iterations_per_checkpoint", "Number of iterations between checkpoints.", cxxopts::value<size_t>()->default_value("0"))
        ("min_distance_change", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("0.0000001"))
        ("distance_change_factor", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("1"))
        ("iterations_change_factor", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("1"))
        ("rows", "Number of rows of the grid.", cxxopts::value<size_t>()->default_value("0"))
        ("columns", "Number of columns of the grid.", cxxopts::value<size_t>()->default_value("0"))
        ("seed", "Randomization seed.", cxxopts::value<size_t>()->default_value(std::to_string(std::rand())))
        ("partition_swaps", "Enable partition swaps.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
        ("randomize", "Randomize the assignment at the start.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
        ("debug", "Enable debugging (use synthetic data).", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
        ("combine_targets", "Combine targets into a single target. If false, one target will be used per pass, with the last target repeating until the end.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
        ("distance_function", "Distance function to use. Options are: Euclidean distance: 0, Cosine Similarity: 1", cxxopts::value<size_t>()->default_value("0"))
        ("targets", "Targets of the SSM. Options are: Aggregate Hierarchy: 0, Highest Parent: 1, Aggregate Hierarchy (4 connected): 2, Highest Parent (4 connected): 3, Partition Neighbourhood: 4, Cell Neighbourhood: 5", cxxopts::value<std::vector<size_t>>()->default_value("4"));

    cxxopts::ParseResult result;
    try { result = options.parse(argc, argv); } catch (const cxxopts::exceptions::exception &exception) {
        std::cerr << "new_ldg: " << exception.what() << std::endl;
        std::cerr << "usage: new_ldg [options] <input_file> ..." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        if (result.count("cores"))
            omp_set_num_threads(result["cores"].as<size_t>());

        auto [data, assignment, dims, depth, num_elements, element_len] = loadDataFromInput<Eigen::VectorXd>(result);
        auto quad_tree = ldg::QuadAssignmentTree<Eigen::VectorXd>(data, assignment, dims.first, dims.second, depth, num_elements, element_len);
        auto schedule = loadScheduleFromInput(result);
        auto sort_options = loadSortOptionsFromInput<Eigen::VectorXd>(result);
        program::run(quad_tree, schedule, sort_options, result["output"].as<std::string>());
    } catch (const std::exception &exception) {
        std::cerr << "new_ldg: " << exception.what() << std::endl;
        std::cerr << "Something went wrong during excecution. Exiting..." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}