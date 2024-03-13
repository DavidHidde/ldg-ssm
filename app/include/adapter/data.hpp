#ifndef LDG_CORE_DATA_HPP
#define LDG_CORE_DATA_HPP

#include <iostream>
#include <Eigen/Core>
#include "app/include/supertiles/util/place/data.hpp"
#include "app/include/ldg/tree_functions.hpp"

namespace adapter
{
    /**
     * Load data from a config file into a vector compatible with the QuadAssignmentTree. Note that all extra cells are set to nullptrs.
     *
     * @tparam VectorType
     * @param config_name
     * @param num_rows
     * @param num_cols
     * @return A tuple of (data, dims, element_len, num_real_elements)
     */
    template<typename VectorType>
    std::tuple<std::vector<std::shared_ptr<VectorType>>, std::pair<size_t, size_t>, size_t, size_t> loadData(std::string const &config_name, size_t num_rows = 0, size_t num_cols = 0)
    {
        auto [data, dim] = supertiles::place::loadData<double>(config_name, std::numeric_limits<size_t>::max(), false);
        size_t data_num_elements = dim.z;
        if (data.size() == 0) {
            std::cerr << "Error: Something went wrong while loading the config!\n";
            exit(EXIT_FAILURE);
        }

        if (num_rows == 0 || num_cols == 0) {
            num_cols = std::pow(2., std::ceil(std::log(data_num_elements) / log(4)));
            num_rows = std::ceil(static_cast<float>(data_num_elements) / static_cast<float>(num_cols));
            std::cout << "No grid dimensions specified, using grid of size " << num_rows << 'x' << num_cols << std::endl;
        }
        size_t grid_num_elements = num_rows * num_cols;
        if (grid_num_elements < data_num_elements) {
            std::cerr << "Error: The config requires " << data_num_elements << " elements, but the grid can only hold " << grid_num_elements << "elements!\n";
            exit(EXIT_FAILURE);
        }

        size_t element_len = dim.x * dim.y;
        size_t required_capacity = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
        std::vector<std::shared_ptr<VectorType>> quad_tree_data(required_capacity);
        // The data array can be viewed as a data_num_elements x element_len matrix. We copy this into vectors.
        for (size_t row = 0; row < data_num_elements; ++row) {
            VectorType vector(element_len);
            for (size_t col = 0; col < element_len; ++col) {
                vector(col) = data[ldg::rowMajorIndex(row, col, element_len)];
            }
            quad_tree_data[row] = std::make_shared<VectorType>(vector);
        }
        // Initialize all aggregates to 0.
        for (auto iterator = quad_tree_data.begin() + grid_num_elements; iterator != quad_tree_data.end(); ++iterator) { *iterator = std::make_shared<Eigen::VectorXd>(Eigen::VectorXd::Zero(element_len)); }

        return {
            quad_tree_data,
            {num_rows, num_cols},
            element_len,
            data_num_elements,
        };
    }
}

#endif //LDG_CORE_DATA_HPP