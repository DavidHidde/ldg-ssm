#ifndef LDG_SSM_INPUT_HPP
#define LDG_SSM_INPUT_HPP

#include <vector>
#include <memory>
#include <cxxopts.hpp>
#include <Eigen/Dense>
#include "app/include/ldg/util/math.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/adapter/data.hpp"
#include "app/include/adapter/storage.hpp"
#include "app/include/ldg/util/metric/distance_function_types.hpp"
#include "app/include/program/run.hpp"
#include "app/include/program/schedule.hpp"
#include "app/include/program/sort_options.hpp"

namespace program
{
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
    std::tuple<std::vector<std::shared_ptr<VectorType>>, std::vector<size_t>, std::pair<size_t, size_t>, size_t, size_t, std::array<size_t, 3>> loadDataFromInput(cxxopts::ParseResult const &result)
    {
        // Debug mode: generate uniform synthetic RGB data
        if (result["debug"].as<bool>()) {
            size_t num_rows = result["rows"].as<size_t>();
            size_t num_cols = result["columns"].as<size_t>();
            auto data = generateUniformRGBData<VectorType>(num_rows, num_cols);
            auto assignment = ldg::createAssignment(data.size());
            return {
                data,
                assignment,
                {num_rows, num_cols},
                std::ceil(std::log2(std::max(num_rows, num_cols))) + 1,
                num_rows * num_cols,
                { 3, 1, 1 }
            };
        }

        if (result.count("config") != 1) {
            std::cerr << "Missing config for non-debug mode operation. Exiting..." << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string config_path = result["config"].as<std::string>();
        InputConfiguration input_config;
        input_config.fromJSONFile(config_path);

        size_t last_separator = config_path.find_last_of("\\/");
        std::string config_dir = last_separator == std::string::npos ? "" : config_path.substr(0, last_separator + 1);
        auto data = adapter::loadData<VectorType>(input_config, config_dir);
        auto [num_rows, num_cols] = input_config.grid_dims;
        std::vector<size_t> assignment = result.count("input") ? adapter::readCompressedAssignment(
            result["input"].as<std::string>(),
            num_rows,
            num_cols,
            input_config.num_elements
        ) : ldg::createAssignment(data.size());

        return {
            data,
            assignment,
            input_config.grid_dims,
            std::ceil(std::log2(std::max(num_rows, num_cols))) + 1,
            input_config.num_elements,
            input_config.data_dims
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
            ldg::mapFunctionTypeToFunction<VectorType>(static_cast<ldg::DistanceFunctionType>(result["distance_function"].as<size_t>()))
        };
    }

    /**
     * Load the export settings from the input arguments.
     * Exits if arguments are invalid or missing.
     *
     * @tparam VectorType
     * @param result
     * @return
     */
    program::ExportSettings loadExportSettingsFromInput(cxxopts::ParseResult const &result)
    {
        std::string output_dir = result["output"].as<std::string>();
        if (!output_dir.empty() && output_dir[output_dir.size() - 1] != '/')
            output_dir += '/';  // Add a trailing slash to the output dir if needed

        return {
            output_dir,
            "", // filename
            result["visualization_config"].as<std::string>(),
            result["log_only"].as<bool>(),
            result["debug"].as<bool>(),
            !result["log_only"].as<bool>() && result["export"].as<bool>(),
            !result["log_only"].as<bool>() && result["visualization_config"].as<std::string>().empty() && result["export"].as<bool>(),
        };
    }
};

#endif //LDG_SSM_INPUT_HPP
