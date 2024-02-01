#include <utility>

#include "../../../include/shared/model/quad_assignment_tree.hpp"

/**
 * @tparam DataType
 * @param grid_num_cols
 * @param projected_num_rows
 * @param projected_num_cols
 * @param offset
 * @param data
 * @param assignment
 * @param index
 */
template<typename DataType>
shared::QuadAssignmentTree<DataType>::RowMajorIterator::RowMajorIterator(
    size_t grid_num_cols,
    size_t projected_num_rows,
    size_t projected_num_cols,
    size_t offset,
    const std::shared_ptr<std::vector<DataType>> &data,
    const std::shared_ptr<std::vector<size_t>> &assignment,
    size_t index
):
    grid_num_cols(grid_num_cols),
    projected_num_rows(projected_num_rows),
    projected_num_cols(projected_num_cols),
    offset(offset),
    data(data),
    assignment(assignment),
    index(index)
{
}

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
shared::QuadAssignmentTree<DataType>::RowMajorIterator shared::QuadAssignmentTree<DataType>::RowMajorIterator::begin()
{
    return shared::QuadAssignmentTree<DataType>::RowMajorIterator{
        grid_num_cols,
        projected_num_rows,
        projected_num_cols,
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
typename shared::QuadAssignmentTree<DataType>::RowMajorIterator shared::QuadAssignmentTree<DataType>::RowMajorIterator::end()
{
    return shared::QuadAssignmentTree<DataType>::RowMajorIterator{
        grid_num_cols,
        projected_num_rows,
        projected_num_cols,
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
DataType &shared::QuadAssignmentTree<DataType>::RowMajorIterator::getValue()
{
    return (*data)[(*assignment)[index]];
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
    const shared::QuadAssignmentTree<DataType>::RowMajorIterator &rhs
)
{
    return currentIndex() == rhs.currentIndex();
}

/**
 * Increment the index.
 * @tparam DataType
 * @return
 */
template<typename DataType>
typename shared::QuadAssignmentTree<DataType>::RowMajorIterator &shared::QuadAssignmentTree<DataType>::RowMajorIterator::operator++()
{
    ++index;
    return *this;
}

/**
 * Construct the quad tree from a data set. Ass
 * @tparam DataType
 * @param data
 */
template<typename DataType>
shared::QuadAssignmentTree<DataType>::QuadAssignmentTree(
    const std::shared_ptr<std::vector<DataType>> &data,
    size_t num_rows,
    size_t num_cols,
    size_t depth
):
    data(data),
    assignment(std::shared_ptr<std::vector<size_t>>(std::vector<size_t>(data->size()))),
    num_rows(num_rows),
    num_cols(num_cols),
    depth(depth)
{
}

/**
 * Get an iterator at a certain height and in a certain quadrant of the tree.
 * @tparam DataType
 * @param height
 * @param quadrant
 * @return
 */
template<typename DataType>
typename shared::QuadAssignmentTree<DataType>::RowMajorIterator shared::QuadAssignmentTree<DataType>::getIteratorAtHeight(
    size_t height,
    Quadrant quadrant
)
{
    size_t start = 0;
    size_t proj_num_rows = num_rows;
    size_t proj_num_cols = num_cols;

    // Go to the start of the desired height grid in the array
    for (size_t curr_height = height; curr_height > 0; --curr_height) {
        start += proj_num_rows * proj_num_cols;
        proj_num_rows /= 2;
        proj_num_cols /= 2;
    }

    // Adjust for quadrant
    if (quadrant != Quadrant::NONE) {
        switch (quadrant) {
            case Quadrant::NORTH_WEST:
                break; // Nothing to do
            case Quadrant::NORTH_EAST:
                start += proj_num_cols / 2;
                break;
            case Quadrant::SOUTH_WEST:
                start += (proj_num_rows / 2) * proj_num_cols;
                break;
            case Quadrant::SOUTH_EAST:
                start += (proj_num_rows / 2) * proj_num_rows + proj_num_cols / 2;
                break;
        }
        proj_num_rows /= 2;
        proj_num_cols /= 2;
    }

    return QuadAssignmentTree::RowMajorIterator{
        num_cols,
        proj_num_rows,
        proj_num_cols,
        start,
        data,
        assignment,
        0
    };
}

/**
 * Get a new shared pointer for the current assignment
 * @tparam DataType
 * @return
 */
template<typename DataType>
std::shared_ptr<std::vector<size_t>> shared::QuadAssignmentTree<DataType>::getAssignment()
{
    return std::shared_ptr<std::vector<size_t>>(assignment);
}

/**
 * @tparam DataType
 * @return
 */
template<typename DataType>
size_t &shared::QuadAssignmentTree<DataType>::getNumCols()
{
    return num_cols;
}

/**
 * @tparam DataType
 * @return
 */
template<typename DataType>
size_t &shared::QuadAssignmentTree<DataType>::getNumRows()
{
    return num_rows;
}

template<typename DataType>
size_t &shared::QuadAssignmentTree<DataType>::getDepth()
{
    return depth;
}
