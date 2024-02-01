#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"

namespace shared
{
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
    RowMajorIterator<DataType>::RowMajorIterator(
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
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t RowMajorIterator<DataType>::currentIndex()
    {
        return offset + (index / projected_num_cols) * grid_num_cols + index % projected_num_cols;
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
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> RowMajorIterator<DataType>::end()
    {
        return RowMajorIterator{
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
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    DataType &RowMajorIterator<DataType>::getValue()
    {
        return (*data)[(*assignment)[index]];
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
        return currentIndex() == rhs.currentIndex();
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
        ++index;
        return *this;
    }

} // shared