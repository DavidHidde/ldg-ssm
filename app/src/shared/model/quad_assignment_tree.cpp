#include <utility>

#include "../../../include/shared/model/quad_assignment_tree.hpp"

namespace shared
{
    /**
     * Construct the quad tree from a data set. Assumes the data is already initialized using a [h0, h1, ...hn] structure.
     *
     * @tparam DataType
     * @param data
     */
    template<typename DataType>
    QuadAssignmentTree<DataType>::QuadAssignmentTree(
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
     * Get the bounds of the data arrays for the given position.
     *
     * @param desired_height
     * @tparam DataType
     * @return <[start, end) of the data array at the given height, [num_rows, num_cols] for the given height>
     */
    template<typename DataType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<DataType>::getBounds(CellPosition &position)
    {
        size_t new_num_rows = num_rows;
        size_t new_num_cols = num_cols;
        size_t offset = 0;
        for (size_t idx = 0; idx < position.height; ++idx) {
            offset += new_num_cols * new_num_rows;
            new_num_cols = ceilDivideByFactor(new_num_cols, 2.);
            new_num_rows = ceilDivideByFactor(new_num_rows, 2.);
        }

        return {
            {
                offset,
                offset + new_num_rows * new_num_cols
            },
            {
                new_num_rows,
                new_num_cols
            }
        };
    }

    /**
     * Get the bounds of the leaf partition in the data arrays for the given position.
     *
     * @param desired_height
     * @tparam DataType
     * @return <[start, end) of the leaf partition at h0, [num_rows, num_cols] for the partition>
     */
    template<typename DataType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<DataType>::getLeafBounds(CellPosition &position)
    {
        float factor = std::pow(2., position.height);
        size_t height_num_cols = ceilDivideByFactor(num_cols, factor);
        auto side_len = size_t(factor);

        size_t offset = rowMajorIndex((position.index % height_num_cols) * side_len, (position.index / height_num_cols) * side_len, num_cols);
        size_t start_row = offset / num_cols;
        size_t start_col = offset % num_cols;
        size_t actual_num_rows = start_row + side_len > num_rows ? num_rows - start_row : side_len;
        size_t actual_num_cols = start_col + side_len > num_cols ? num_cols - start_col : side_len;

        return {
            {
                offset,
                rowMajorIndex(start_row + actual_num_rows, start_col + actual_num_cols, num_cols)
            },
            {
                actual_num_rows,
                actual_num_cols
            }
        };
    }

    /**
     * Get a value at a position in the tree. If it is out-of-bounds, returns nullptr.
     *
     * @tparam DataType
     * @param position
     * @return
     */
    template<typename DataType>
    DataType &QuadAssignmentTree<DataType>::getValue(CellPosition &position)
    {
        auto bounds = getBounds(position);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;
        return index < start_end.second ? (*data)[(*assignment)[index]] : nullptr;
    }

    /**
     * Compute and update the aggregates of the quad tree.
     * For this we assume we can perform basic arithmetic functions on DataType.
     *
     * @tparam DataType
     */
    template<typename DataType>
    void QuadAssignmentTree<DataType>::computeAggregates()
    {
        size_t curr_num_rows = ceilDivideByFactor(num_rows, 2.);
        size_t curr_num_cols = ceilDivideByFactor(num_cols, 2.);

        // Compute the aggregates bottom-up
        for (size_t height = 1; height < depth; ++height) {
            for (size_t idx = 0; idx < curr_num_rows * curr_num_cols; ++idx) {
                CellPosition position{ height, idx };
                TreeWalker<DataType> walker(position, num_rows, num_cols, *this);
                float count = 0.;
                DataType aggregate;
                for (DataType child: walker.getChildrenValues()) {
                    if (child != nullptr) {
                        ++count;
                        aggregate += child;
                    }
                }
                DataType &node = getValue(position);
                node = aggregate;
            }

            curr_num_rows = ceilDivideByFactor(curr_num_rows, 2.);
            curr_num_cols = ceilDivideByFactor(curr_num_cols, 2.);
        }
    }

    /**
     * Get a new shared pointer for the current assignment.
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    std::shared_ptr<std::vector<size_t>> QuadAssignmentTree<DataType>::getAssignment()
    {
        return std::shared_ptr<std::vector<size_t>>(assignment);
    }

    /**
     * Get a new shared pointer for the current data.
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    std::shared_ptr<std::vector<DataType>> QuadAssignmentTree<DataType>::getData()
    {
        return std::shared_ptr<std::vector<DataType>>(data);
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t &QuadAssignmentTree<DataType>::getNumCols()
    {
        return num_cols;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t &QuadAssignmentTree<DataType>::getNumRows()
    {
        return num_rows;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t &QuadAssignmentTree<DataType>::getDepth()
    {
        return depth;
    }
}
