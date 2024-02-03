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
        node{height, 0},
        offset(0),
        num_rows(quad_tree->getNumRows() * std::pow(2, -height)),
        num_cols(quad_tree->getNumCols() * std::pow(2, -height)),
        quad_tree(quad_tree)
    {
    }

    /**
     * Create an iterator starting from a certain position.
     *
     * @tparam DataType
     * @param position
     * @param quad_tree
     */
    template<typename DataType>
    RowMajorIterator<DataType>::RowMajorIterator(CellPosition &position, QuadAssignmentTree<DataType> &quad_tree):
        node(position),
        offset(0),
        num_rows(quad_tree->getNumRows() * std::pow(2, -position.height)),
        num_cols(quad_tree->getNumCols() * std::pow(2, -position.height)),
        quad_tree(quad_tree)
    {
    }

    /**
     * Create an iterator starting at a given position given an offset and a number of rows and columns.
     * This allows for smaller partitions of a height array to be iterated over.
     *
     * @tparam DataType
     * @param position
     * @param quad_tree
     * @param offset
     * @param num_rows
     * @param num_cols
     */
    template<typename DataType>
    RowMajorIterator<DataType>::RowMajorIterator(
        CellPosition &position,
        QuadAssignmentTree<DataType> &quad_tree,
        size_t offset,
        size_t num_rows,
        size_t num_cols
    ):
        node(position),
        quad_tree(quad_tree),
        offset(offset),
        num_rows(num_rows),
        num_cols(num_cols)
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
            CellPosition{node.height, offset},
            offset,
            num_rows,
            num_cols,
            quad_tree
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
            CellPosition{node.height, offset + num_rows * num_cols},
            offset,
            num_rows,
            num_cols,
            quad_tree
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
        ++node.index;
        return *this;
    }
} // shared