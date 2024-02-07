#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"

namespace shared
{
    /**
     * Construct an iterator for the whole array of a given height.
     *
     * @tparam DataType
     * @param height
     */
    template<typename DataType>
    RowMajorIterator<DataType>::RowMajorIterator(size_t height, QuadAssignmentTree<DataType> &quad_tree):
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
     * @tparam DataType
     * @param position
     * @param quad_tree
     * @param offset    0 by default.
     * @param num_rows  Number of rows at the current height by default.
     * @param num_cols  Number of cols at the current height by default.
     * @param height_num_rows   Number of rows at the current height by default.
     * @param height_num_cols   Number of cols at the current height by default.
     */
    template<typename DataType>
    RowMajorIterator<DataType>::RowMajorIterator(
        CellPosition &position,
        QuadAssignmentTree<DataType> &quad_tree,
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
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> RowMajorIterator<DataType>::begin()
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
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> RowMajorIterator<DataType>::end()
    {
        return RowMajorIterator{
            CellPosition{
                node.height,
                rowMajorIndex(
                    offset / height_num_cols + num_rows,
                    offset % height_num_cols + num_cols,
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
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    DataType &RowMajorIterator<DataType>::getValue()
    {
        return quad_tree->getValue(node);
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    DataType &RowMajorIterator<DataType>::getPosition()
    {
        return node;
    }

    /**
     * Two iterators are equal if they point towards the same data set index.
     *
     * @tparam DataType
     * @param lhs
     * @param rhs
     * @return
     */
    template<typename DataType>
    bool RowMajorIterator<DataType>::operator==(
        const RowMajorIterator<DataType> &rhs
    )
    {
        return node.index == rhs.node.index && node.height == rhs.node.height;
    }

    /**
     * Increment the index.
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> &RowMajorIterator<DataType>::operator++()
    {
        // Move to the next row in the current partition if needed.
        if ((node.index + 1) % height_num_cols == 0) {
            node.index = rowMajorIndex(index / height_num_cols + 1, offset % height_num_cols, height_num_cols);
        } else {
            ++node.index;
        }

        return *this;
    }
} // shared