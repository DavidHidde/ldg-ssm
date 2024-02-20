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
     * @tparam VectorType Type of the underlying data.
     */
    template<typename VectorType>
    class RowMajorIterator
    {
        CellPosition node;  // The current node pointer.
        size_t offset;      // Offset of the start in the current height array.
        size_t num_rows;    // Number of rows to iterate over.
        size_t num_cols;    // Number of columns to iterate over.

        size_t height_num_rows; // Number of rows in the current height array
        size_t height_num_cols; // Number of cols in the current height array

        QuadAssignmentTree<VectorType> &quad_tree;

    public:
        RowMajorIterator(size_t height, QuadAssignmentTree<VectorType> &quad_tree);

        RowMajorIterator(
            CellPosition position,
            QuadAssignmentTree<VectorType> &quad_tree,
            size_t offset = 0,
            size_t num_rows = 0,
            size_t num_cols = 0,
            size_t height_num_rows = 0,
            size_t height_num_cols = 0
        );

        RowMajorIterator begin();

        RowMajorIterator end();

        std::shared_ptr<VectorType> getValue();

        CellPosition &getPosition();

        bool operator==(RowMajorIterator const &rhs);

        bool operator!=(RowMajorIterator const &rhs);

        RowMajorIterator &operator++();
    };

    /**
     * Construct an iterator for the whole array of a given height.
     *
     * @tparam VectorType
     * @param height
     */
    template<typename VectorType>
    RowMajorIterator<VectorType>::RowMajorIterator(size_t height, QuadAssignmentTree<VectorType> &quad_tree):
        node{ height, 0 },
        quad_tree(quad_tree),
        offset(0),
        num_rows(ceilDivideByPowerTwo(quad_tree.getNumRows(), height)),
        num_cols(ceilDivideByPowerTwo(quad_tree.getNumCols(), height)),
        height_num_rows(num_rows),
        height_num_cols(num_cols)
    {
    }

    /**
     * Create an iterator starting at a given position given an offset and a number of rows and columns.
     * This allows for smaller partitions of a height array to be iterated over.
     *
     * @tparam VectorType
     * @param position
     * @param quad_tree
     * @param offset    0 by default.
     * @param num_rows  Number of rows at the current height by default.
     * @param num_cols  Number of cols at the current height by default.
     * @param height_num_rows   Number of rows at the current height by default.
     * @param height_num_cols   Number of cols at the current height by default.
     */
    template<typename VectorType>
    RowMajorIterator<VectorType>::RowMajorIterator(
        CellPosition position,
        QuadAssignmentTree<VectorType> &quad_tree,
        size_t offset,
        size_t num_rows,
        size_t num_cols,
        size_t height_num_rows,
        size_t height_num_cols
    ):
        node(position),
        quad_tree(quad_tree),
        offset(offset),
        num_rows(num_rows > 0 ? num_rows : ceilDivideByPowerTwo(quad_tree.getNumRows(), position.height)),
        num_cols(num_cols > 0 ? num_cols : ceilDivideByPowerTwo(quad_tree.getNumCols(), position.height)),
        height_num_rows(height_num_rows > 0 ? height_num_rows : ceilDivideByPowerTwo(quad_tree.getNumRows(), position.height)),
        height_num_cols(height_num_cols > 0 ? height_num_cols : ceilDivideByPowerTwo(quad_tree.getNumCols(), position.height))
    {
    }

    /**
     * The begin iterator starts at index 0
     *
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    RowMajorIterator<VectorType> RowMajorIterator<VectorType>::begin()
    {
        return RowMajorIterator{
            CellPosition{ node.height, offset },
            quad_tree,
            offset,
            num_rows,
            num_cols,
            height_num_rows,
            height_num_cols
        };
    }

    /**
     * The iterator ends when all rows and columns have been traversed.
     *
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    RowMajorIterator<VectorType> RowMajorIterator<VectorType>::end()
    {
        return RowMajorIterator{
            CellPosition{
                node.height,
                rowMajorIndex(
                    offset / height_num_cols + num_rows,
                    offset % height_num_cols,
                    height_num_cols
                ),
            },
            quad_tree,
            offset,
            num_rows,
            num_cols,
            height_num_rows,
            height_num_cols
        };
    }

    /**
     * Get the data value the iterator is currently pointing at.
     *
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    std::shared_ptr<VectorType> RowMajorIterator<VectorType>::getValue()
    {
        return quad_tree.getValue(node);
    }

    /**
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    CellPosition &RowMajorIterator<VectorType>::getPosition()
    {
        return node;
    }

    /**
     * Two iterators are equal if they point towards the same data set index.
     *
     * @tparam VectorType
     * @param lhs
     * @param rhs
     * @return
     */
    template<typename VectorType>
    bool RowMajorIterator<VectorType>::operator==(const RowMajorIterator<VectorType> &rhs)
    {
        return node.index == rhs.node.index && node.height == rhs.node.height;
    }

    /**
     * Two iterators are not equal if they point towards a different data set index.
     *
     * @tparam VectorType
     * @param lhs
     * @param rhs
     * @return
     */
    template<typename VectorType>
    bool RowMajorIterator<VectorType>::operator!=(const RowMajorIterator<VectorType> &rhs)
    {
        return node.index != rhs.node.index || node.height != rhs.node.height;
    }

    /**
     * Increment the index.
     *
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    RowMajorIterator<VectorType> &RowMajorIterator<VectorType>::operator++()
    {
        // Move to the next row in the current partition if needed.
        if ((node.index % height_num_cols - offset % height_num_cols) + 1 < num_cols) {
            ++node.index;
        } else {
            node.index = rowMajorIndex(node.index / height_num_cols + 1, offset % height_num_cols, height_num_cols);
        }

        return *this;
    }
} // shared

#endif //LDG_CORE_ROW_MAJOR_ITERATOR_HPP
