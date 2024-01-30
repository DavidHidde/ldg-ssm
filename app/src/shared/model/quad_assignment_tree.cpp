#include "../../../include/shared/model/quad_assignment_tree.hpp"

/**
 * Compute the current array index of the iterator.
 * @tparam DataType
 * @return
 */
template<typename DataType>
size_t shared::QuadAssignmentTree<DataType>::RowMajorIterator::currentIndex()
{
    return offset + (index / projected_num_cols) * grid_num_cols + index % projected_num_cols;
}

/**
 * The begin iterator starts at index 0
 * @tparam DataType
 * @return
 */
template<typename DataType>
shared::QuadAssignmentTree::RowMajorIterator shared::QuadAssignmentTree<DataType>::RowMajorIterator::begin()
{
    return shared::QuadAssignmentTree::RowMajorIterator{
        grid_num_cols,
        projected_num_rows,
        projected_num_cols
        offset,
        data,
        assignment,
        0
    };
}

/**
 * The iterator ends when all rows and columns have been traversed.
 * @tparam DataType
 * @return
 */
template<typename DataType>
shared::QuadAssignmentTree::RowMajorIterator shared::QuadAssignmentTree<DataType>::RowMajorIterator::end()
{
    return shared::QuadAssignmentTree::RowMajorIterator{
        grid_num_cols,
        projected_num_rows,
        projected_num_cols
        offset,
        data,
        assignment,
        projected_num_rows * projected_num_cols
    };
}

/**
 * Get the data value the iterator is currently pointing at.
 * @tparam DataType
 * @return
 */
template<typename DataType>
DataType shared::QuadAssignmentTree<DataType>::RowMajorIterator::getValue()
{
    return data[assignment[index]];
}

/**
 * Two iterators are equal if they point towards the same data set index.
 * @tparam DataType
 * @param lhs
 * @param rhs
 * @return
 */
template<typename DataType>
bool shared::QuadAssignmentTree<DataType>::RowMajorIterator::operator==(
    const shared::QuadAssignmentTree::RowMajorIterator &lhs,
    const shared::QuadAssignmentTree::RowMajorIterator &rhs
)
{
    return lhs.currentIndex() == rhs.currentIndex();
}

/**
 * Increment the index.
 * @tparam DataType
 * @return
 */
template<typename DataType>
shared::QuadAssignmentTree::RowMajorIterator &shared::QuadAssignmentTree<DataType>::RowMajorIterator::operator++()
{
    ++index;
    return *this;
}
