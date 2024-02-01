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
     * Get an iterator at a certain height and in a certain quadrant of the tree.
     *
     * @tparam DataType
     * @param height
     * @param quadrant
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> QuadAssignmentTree<DataType>::getIteratorAtHeight(
        size_t height,
        Quadrant quadrant
    )
    {
        auto bounds = getHeightBounds(height);
        size_t start = bounds.first.first;
        size_t proj_num_rows = bounds.second.first;
        size_t proj_num_cols = bounds.second.second;

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

        return RowMajorIterator<DataType>{
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
     * Get the bounds and size of the assigment/data array at a certain height
     *
     * @tparam DataType
     * @param height
     * @return A pair of two pairs:
     *  1. The [start, end) bounds in the assignment/data grid.
     *  2. The size of the assignment/data grid, expressed in [num_rows, num_cols].
     */
    template<typename DataType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<DataType>::getHeightBounds(
        size_t height
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

        return std::make_pair(
            std::pair<size_t, size_t>(start, start + proj_num_rows * proj_num_cols),
            std::pair<size_t, size_t>(proj_num_rows, proj_num_cols)
        );
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
