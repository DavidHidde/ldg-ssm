#ifndef IMPROVED_LDG_ASSIGNMENT_HPP
#define IMPROVED_LDG_ASSIGNMENT_HPP

#include <array>

namespace shared
{
    /**
     * A POD num_rows x num_cols grid assignment class, with the assignment stored as indices of a data set.
     * @tparam DataType The data set data type.
     */
    template<typename DataType>
    struct Assignment
    {
        const size_t num_rows;
        const size_t num_cols;
        array<size_t> assigned_indices;  // Row-major assignment of indices of data.
        std::shared_ptr<array<DataType>> data;
    };
}

#endif //IMPROVED_LDG_ASSIGNMENT_HPP
