#ifndef IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
#define IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP

#include "app/include/shared/util/math.hpp"
#include "cell_position.hpp"

#include <vector>
#include <cstddef>
#include <memory>
#include <cmath>

namespace shared
{
    /**
     * Quad tree of the data. Uses a flat row-major data array for all heights of the tree, which should already exist.
     * Uses an assignment array to determine the grid assignment.
     * @tparam DataType The data type of the grid.
     */
    template<typename DataType>
    class QuadAssignmentTree
    {
        size_t num_rows;
        size_t num_cols;
        size_t depth;

        std::shared_ptr<std::vector<std::shared_ptr<DataType>>> data;
        std::shared_ptr<std::vector<size_t>> assignment;

    public:
        QuadAssignmentTree(
            const std::shared_ptr<std::vector<std::shared_ptr<DataType>>> &data,
            const std::shared_ptr<std::vector<size_t>> &assignment,
            size_t num_rows,
            size_t num_cols,
            size_t depth
        );

        size_t &getDepth();

        size_t &getNumRows();

        size_t &getNumCols();

        std::shared_ptr<std::vector<size_t>> getAssignment();

        std::shared_ptr<std::vector<std::shared_ptr<DataType>>> getData();

        std::shared_ptr<DataType> getValue(CellPosition position);

        bool setValue(CellPosition position, DataType &value);

        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> getBounds(CellPosition position);

        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> getLeafBounds(CellPosition position);
    };

    /**
     * Construct the quad tree from a data set. Assumes the data is already initialized using a [h0, h1, ...hn] structure.
     * The assignment is initialized to the index of the tree.
     *
     * @tparam DataType
     * @param data
     * @param num_rows
     * @param num_cols
     * @param depth
     */
    template<typename DataType>
    QuadAssignmentTree<DataType>::QuadAssignmentTree(
        const std::shared_ptr<std::vector<std::shared_ptr<DataType>>> &data,
        const std::shared_ptr<std::vector<size_t>> &assignment,
        size_t num_rows,
        size_t num_cols,
        size_t depth
    ):
        data(data),
        assignment(assignment),
        num_rows(num_rows),
        num_cols(num_cols),
        depth(depth)
    {

    }

    /**
     * Get the bounds of the data arrays for the given position.
     *
     * @param position
     * @tparam DataType
     * @return <[start, end) of the data array at the given height, [num_rows, num_cols] for the given height>
     */
    template<typename DataType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<DataType>::getBounds(CellPosition position)
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
     * @param position
     * @tparam DataType
     * @return <[start, end) of the leaf partition at h0, [num_rows, num_cols] for the partition>
     */
    template<typename DataType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<DataType>::getLeafBounds(CellPosition position)
    {
        float factor = std::pow(2., position.height);
        size_t height_num_cols = ceilDivideByFactor(num_cols, factor);
        auto side_len = size_t(factor);

        size_t offset = rowMajorIndex(
            (position.index % height_num_cols) * side_len,
            (position.index / height_num_cols) * side_len,
            num_cols
        );
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
    std::shared_ptr<DataType> QuadAssignmentTree<DataType>::getValue(CellPosition position)
    {
        auto bounds = getBounds(position);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;
        return index < start_end.second ? (*data)[(*assignment)[index]] : nullptr;
    }

    /**
     * Set the value at a given position.
     *
     * @tparam DataType
     * @param position
     * @return True if setting the value worked, false if not.
     */
    template<typename DataType>
    bool QuadAssignmentTree<DataType>::setValue(CellPosition position, DataType &value)
    {
        auto bounds = getBounds(position);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;

        if (index < start_end.second) {
            *(*data)[(*assignment)[index]] = std::move(value);
            return true;
        }

        return false;
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
    std::shared_ptr<std::vector<std::shared_ptr<DataType>>> QuadAssignmentTree<DataType>::getData()
    {
        return std::shared_ptr<std::vector<std::shared_ptr<DataType>>>(data);
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

#endif //IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
