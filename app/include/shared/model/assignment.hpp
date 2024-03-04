#ifndef IMPROVED_LDG_ASSIGNMENT_HPP
#define IMPROVED_LDG_ASSIGNMENT_HPP

#include <cstddef>
#include <memory>
#include <vector>

namespace shared
{
    /**
     * A POD num_rows x num_cols grid assignment class, with the assignment stored as indices of a data set.
     * @tparam VectorType The data set data type.
     */
    template<typename VectorType>
    struct Assignment
    {
        const size_t num_rows;
        const size_t num_cols;
        std::vector<size_t> assigned_indices;  // Row-major assignment of indices of data.
        std::shared_ptr<std::vector<VectorType>> data;
    };
}

#endif //IMPROVED_LDG_ASSIGNMENT_HPP
