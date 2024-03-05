#ifndef LDG_CORE_DATA_HPP
#define LDG_CORE_DATA_HPP

#include <iostream>
#include <Eigen/Core>
#include "app/include/supertiles/util/place/data.hpp"
#include "app/include/shared/tree_functions.hpp"

namespace adapter
{
    /**
     * Load data from a config file into a vector compatible with the QuadAssignmentTree. Note that all extra cells are set to nullptrs.
     *
     * @param config_name
     * @param num_rows
     * @param num_cols
     * @return A tuple of (data, element_len, num_real_elements)
     */
    std::tuple<std::vector<std::shared_ptr<Eigen::VectorXd>>, size_t, size_t> loadData(std::string const config_name, size_t num_rows, size_t num_cols)
    {
        auto [data, dim] = supertiles::place::loadData<double>(config_name, std::numeric_limits<size_t>::max(), false);
        size_t data_num_elements = dim.z;
        size_t grid_num_elements = num_rows * num_cols;

        if (data.size() == 0) {
            std::cerr << "Error: Something went wrong while loading the config!\n";
            exit(-1);
        }
        if (grid_num_elements < data_num_elements) {
            std::cerr << "Error: The config requires " << data_num_elements << " elements, but the grid can only hold " << grid_num_elements << "elements!\n";
            exit(-1);
        }

        size_t element_len = dim.x * dim.y;
        size_t required_capacity = shared::determineRequiredArrayCapacity(num_rows, num_cols);
        std::vector<std::shared_ptr<Eigen::VectorXd>> quad_tree_data(required_capacity);
        // The data array can be viewed as a data_num_elements x element_len matrix. We copy this into vectors.
        for (size_t row = 0; row < data_num_elements; ++row) {
            Eigen::VectorXd vector(element_len);
            for (size_t col = 0; col < element_len; ++col) {
                vector(col) = data[shared::rowMajorIndex(row, col, element_len)];
            }
            quad_tree_data[row] = std::make_shared<Eigen::VectorXd>(vector);
        }
        // Initialize all aggregates to 0.
        for (auto iterator = quad_tree_data.begin() + grid_num_elements; iterator != quad_tree_data.end(); ++iterator) {
            *iterator = std::make_shared<Eigen::VectorXd>(Eigen::VectorXd::Zero(element_len));
        }

        return {
            quad_tree_data,
            element_len,
            data_num_elements
        };
    }
}

#endif //LDG_CORE_DATA_HPP
