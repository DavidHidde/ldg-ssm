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
        size_t factor = std::pow(0.5, position.height);
        size_t new_num_rows = num_rows * factor;
        size_t new_num_cols = num_cols * factor;
        size_t offset = num_rows * num_cols * ((1. - std::pow(
            0.25,
            position.height
        )) / (1. - 0.25)); // Geometric summation.
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
        size_t curr_idx = 0;
        size_t next_height_idx = num_rows * num_cols;
        size_t curr_num_rows = num_rows;
        size_t curr_num_cols = num_cols;

        // Compute the aggregates bottom-up
        for (size_t height = 0; height < depth - 1; ++height) {
            size_t num_pairs = (curr_num_rows * curr_num_cols) / 4;
            size_t num_pairs_per_row = curr_num_cols / 2;
            for (size_t pair_idx = 0; pair_idx < num_pairs; ++pair_idx) {
                // Load data
                DataType tl = (*data)[(*assignment)[curr_idx]];                 // Top left
                DataType tr = (*data)[(*assignment)[curr_idx + 1]];             // Top right
                DataType bl = (*data)[(*assignment)[curr_idx + num_cols]];      // Bottom left
                DataType br = (*data)[(*assignment)[curr_idx + num_cols + 1]];  // Bottom right

                // Aggregate and normalize, ignoring VOID (nullptr) tiles
                double count = 0;
                DataType aggregate;
                if (tl != nullptr) {
                    aggregate += tl;
                    ++count;
                }
                if (tr != nullptr) {
                    aggregate += tr;
                    ++count;
                }
                if (bl != nullptr) {
                    aggregate += bl;
                    ++count;
                }
                if (br != nullptr) {
                    aggregate += br;
                    ++count;
                }
                (*data)[(*assignment)[next_height_idx]] = aggregate / count;

                // Update for next step. The curr_idx needs to move in blocks of 4.
                ++next_height_idx;
                curr_idx += 2;
                if ((pair_idx + 1) % num_pairs_per_row == 0)
                    curr_idx += curr_num_cols;
            }
            curr_num_rows /= 2;
            curr_num_cols /= 2;
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
