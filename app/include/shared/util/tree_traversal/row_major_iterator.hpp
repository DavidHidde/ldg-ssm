#ifndef LDG_CORE_ROW_MAJOR_ITERATOR_HPP
#define LDG_CORE_ROW_MAJOR_ITERATOR_HPP

#include "app/include/shared/model/quad_assignment_tree.hpp"
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
        CellPosition node;
        size_t offset;
        size_t num_rows;
        size_t num_cols;

        QuadAssignmentTree<DataType> *quad_tree;

    public:
        RowMajorIterator(size_t height, QuadAssignmentTree<DataType> &quad_tree);

        RowMajorIterator(
            CellPosition &position,
            QuadAssignmentTree<DataType> &quad_tree
        );

        RowMajorIterator(
            CellPosition &position,
            QuadAssignmentTree<DataType> &quad_tree,
            size_t offset,
            size_t num_rows,
            size_t num_cols
        );

        RowMajorIterator begin();

        RowMajorIterator end();

        DataType &getValue();

        bool operator==(RowMajorIterator const &rhs);

        RowMajorIterator &operator++();
    };
} // shared

#endif //LDG_CORE_ROW_MAJOR_ITERATOR_HPP
