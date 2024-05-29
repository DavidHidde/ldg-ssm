#ifndef LDG_CORE_DATA_HPP
#define LDG_CORE_DATA_HPP

#include <iostream>
#include <Eigen/Core>
#include "app/include/ldg/util/tree_functions.hpp"

namespace adapter
{
    /**
     * Load data from a JSON config file into a vector compatible with the QuadAssignmentTree. Note that all extra cells are set to nullptrs.
     *
     * @tparam VectorType
     * @param config
     * @return
     */
    template<typename VectorType>
    std::vector<std::shared_ptr<VectorType>> loadData(program::InputConfiguration &config, std::string config_dir)
    {
        // Check dims
        auto [num_rows, num_cols] = config.grid_dims;
        size_t grid_num_elements = num_rows * num_cols;
        size_t element_len = config.data_dims[0] * config.data_dims[1] * config.data_dims[2];
        if (grid_num_elements < config.num_elements) {
            std::cerr << "Error: The config requires " << config.num_elements << " elements, but the grid can only hold " << grid_num_elements << "elements!\n";
            exit(EXIT_FAILURE);
        }

        // Load the data
        std::vector<double> data(element_len * grid_num_elements, 0.);
        if (!helper::bzip_decompress(data, config_dir + config.data_path)) {
            std::cerr << "Error: Unable to load data from file \"" << config_dir + config.data_path << "\"\n";
            exit(EXIT_FAILURE);
        }

        // Copy data over from data buffer into quad tree vector
        size_t required_capacity = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
        std::vector<std::shared_ptr<VectorType>> quad_tree_data(required_capacity, nullptr);
        // The data array can be viewed as a data_num_elements x element_len matrix. We copy this into vectors.
        for (size_t row = 0; row < config.num_elements; ++row) {
            VectorType vector(element_len);
            for (size_t col = 0; col < element_len; ++col) {
                vector(col) = data[ldg::rowMajorIndex(row, col, element_len)];
            }
            quad_tree_data[row] = std::make_shared<VectorType>(vector);
        }
        // Initialize all aggregates to 0.
        for (auto iterator = quad_tree_data.begin() + grid_num_elements; iterator != quad_tree_data.end(); ++iterator) {
            *iterator = std::make_shared<Eigen::VectorXd>(Eigen::VectorXd::Zero(element_len));
        }

        return quad_tree_data;
    }
}

#endif //LDG_CORE_DATA_HPP