#ifndef LDG_CORE_ROW_MAJOR_ITERATOR_HPP
#define LDG_CORE_ROW_MAJOR_ITERATOR_HPP

#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/math.hpp"
#include <vector>
#include <cstddef>
#include <memory>

namespace shared
{
    /**
     * Simple iterator for iterating over row-major arrays in the quad tree.
     * @tparam DataType Type of the underlying data.
     */
    template<typename DataType>
    class RowMajorIterator
    {
        CellPosition node;  // The current node pointer.
        size_t offset;      // Offset of the start in the current height array.
        size_t num_rows;    // Number of rows to iterate over.
        size_t num_cols;    // Number of columns to iterate over.

        size_t height_num_rows; // Number of rows in the current height array
        size_t height_num_cols; // Number of cols in the current height array

        QuadAssignmentTree<DataType> *quad_tree;

    public:
        RowMajorIterator(size_t height, QuadAssignmentTree<DataType> &quad_tree);

        RowMajorIterator(
            CellPosition &position,
            QuadAssignmentTree<DataType> &quad_tree,
            size_t offset = 0,
            size_t num_rows = 0,
            size_t num_cols = 0,
            size_t height_num_rows = 0,
            size_t height_num_cols = 0
        );

        RowMajorIterator begin();

        RowMajorIterator end();

        DataType &getValue();

        CellPosition &getPosition();

        bool operator==(RowMajorIterator const &rhs);

        RowMajorIterator &operator++();
    };
} // shared

#endif //LDG_CORE_ROW_MAJOR_ITERATOR_HPP
