#ifndef IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
#define IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP

#include "app/include/ldg/util/math.hpp"
#include "cell_position.hpp"
#include "parent_type.hpp"

#include <vector>
#include <cstddef>
#include <memory>
#include <cmath>

namespace ldg
{
    /**
     * Quad tree of the data. Uses a flat row-major data array for all heights of the tree, which should already exist.
     * Uses an assignment array to determine the grid assignment.
     * @tparam VectorType The data type of the grid.
     */
    template<typename VectorType>
    class QuadAssignmentTree
    {
        size_t num_rows;
        size_t num_cols;
        size_t depth;
        size_t num_real_elements;
        std::array<size_t, 3> data_dims;

        ParentType parent_type;
        std::vector<std::shared_ptr<VectorType>> data;
        std::vector<size_t> assignment;
        std::vector<std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>>> bounds_cache;

    public:
        QuadAssignmentTree(
            const std::vector<std::shared_ptr<VectorType>> &data,
            const std::vector<size_t> &assignment,
            size_t num_rows,
            size_t num_cols,
            size_t depth,
            size_t num_real_elements,
            std::array<size_t, 3> data_dims,
            ParentType aggregation_type
        );

        size_t getDepth() const;

        size_t getNumRows() const;

        size_t getNumCols() const;

        size_t getNumRealElements() const;

        std::array<size_t, 3> getDataDims() const;

        size_t getDataElementLen() const;

        ParentType getParentType() const;

        std::vector<size_t> &getAssignment();

        std::vector<std::shared_ptr<VectorType>> &getData();

        std::shared_ptr<VectorType> getValue(CellPosition position);

        bool setValue(CellPosition position, VectorType *value);

        size_t getAssignmentValue(CellPosition position);

        bool setAssignmentValue(CellPosition position, const size_t &value);

        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> getBounds(size_t height);

        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> getLeafBounds(CellPosition position);
    };

    /**
     * Construct the quad tree from a data set. Assumes the data is already initialized using a [h0, h1, ...hn] structure.
     * The assignment is initialized to the index of the tree.
     *
     * @tparam VectorType
     * @param data
     * @param assignment
     * @param num_rows
     * @param num_cols
     * @param depth
     * @param num_real_elements
     * @param data_element_len
     */
    template<typename VectorType>
    QuadAssignmentTree<VectorType>::QuadAssignmentTree(
        const std::vector<std::shared_ptr<VectorType>> &data,
        const std::vector<size_t> &assignment,
        size_t num_rows,
        size_t num_cols,
        size_t depth,
        size_t num_real_elements,
        std::array<size_t, 3> data_dims,
        ParentType aggregation_type
    ):
        data(data),
        assignment(assignment),
        num_rows(num_rows),
        num_cols(num_cols),
        depth(depth),
        num_real_elements(num_real_elements),
        data_dims(data_dims),
        parent_type(aggregation_type)
    {
        // Generate the bounds cache
        bounds_cache.reserve(depth);
        size_t new_num_rows = num_rows;
        size_t new_num_cols = num_cols;
        size_t offset = 0;
        for (size_t height = 0; height < depth; ++height) {
            bounds_cache.push_back(std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> {
                {
                    offset,
                    offset + new_num_rows * new_num_cols
                },
                {
                    new_num_rows,
                    new_num_cols
                }
            });
            offset += new_num_cols * new_num_rows;
            new_num_cols = ceilDivideByFactor(new_num_cols, 2.);
            new_num_rows = ceilDivideByFactor(new_num_rows, 2.);
        }
    }

    /**
     * Get the bounds of the data arrays for the given position.
     *
     * @param height
     * @tparam VectorType
     * @return <[start, end) of the data array at the given height, [num_rows, num_cols] for the given height>
     */
    template<typename VectorType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<VectorType>::getBounds(size_t height)
    {
        return bounds_cache[height];
    }

    /**
     * Get the bounds of the leaf partition in the data arrays for the given position.
     *
     * @param position
     * @tparam VectorType
     * @return <[start, end) of the leaf partition at h0, [num_rows, num_cols] for the partition>
     */
    template<typename VectorType>
    std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> QuadAssignmentTree<VectorType>::getLeafBounds(CellPosition position)
    {
        float factor = std::pow(2., position.height);
        size_t height_num_cols = ceilDivideByFactor(num_cols, factor);
        auto side_len = size_t(factor);

        size_t offset = rowMajorIndex(
            (position.index / height_num_cols) * side_len,
            (position.index % height_num_cols) * side_len,
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
     * @tparam VectorType
     * @param position
     * @return
     */
    template<typename VectorType>
    std::shared_ptr<VectorType> QuadAssignmentTree<VectorType>::getValue(CellPosition position)
    {
        auto bounds = getBounds(position.height);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;
        return index < start_end.second ? data[assignment[index]] : nullptr;
    }

    /**
     * Set the value at a given position.
     *
     * @tparam VectorType
     * @param position
     * @param value
     * @return True if setting the value worked, false if not.
     */
    template<typename VectorType>
    bool QuadAssignmentTree<VectorType>::setValue(CellPosition position, VectorType *value)
    {
        auto bounds = getBounds(position.height);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;

        if (index < start_end.second) {
            if (value == nullptr) {
                data[assignment[index]] = nullptr;
            } else if (data[assignment[index]] == nullptr) {
                data[assignment[index]] = std::make_shared<VectorType>(*value);
            } else {
                *data[assignment[index]] = std::move(*value);
            }
            return true;
        }

        return false;
    }

    /**
     * Get the assignment value at a position in the tree. This is not safe for out of bounds values.
     *
     * @tparam VectorType
     * @param position
     * @return
     */
    template<typename VectorType>
    size_t QuadAssignmentTree<VectorType>::getAssignmentValue(CellPosition position)
    {
        auto bounds = getBounds(position.height);
        auto start_end = bounds.first;
        size_t index = start_end.first + position.index;
        return assignment[index];
    }

    /**
     * Set the assignment at a given position.
     *
     * @tparam VectorType
     * @param position
     * @param value
     * @return True if setting the value worked, false if not.
     */
    template<typename VectorType>
    bool QuadAssignmentTree<VectorType>::setAssignmentValue(const CellPosition position, const size_t &value)
    {
        auto [start_end, dims] = getBounds(position.height);

        if (size_t index = start_end.first + position.index; index < start_end.second) {
            assignment[index] = value;
            return true;
        }

        return false;
    }

    /**
     * Get the current assignment.
     *
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    std::vector<size_t> &QuadAssignmentTree<VectorType>::getAssignment()
    {
        return assignment;
    }

    /**
     * Get the current data.
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    std::vector<std::shared_ptr<VectorType>> &QuadAssignmentTree<VectorType>::getData()
    {
        return data;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t QuadAssignmentTree<DataType>::getNumCols() const
    {
        return num_cols;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t QuadAssignmentTree<DataType>::getNumRows() const
    {
        return num_rows;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t QuadAssignmentTree<DataType>::getDepth() const
    {
        return depth;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t QuadAssignmentTree<DataType>::getNumRealElements() const
    {
        return num_real_elements;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    std::array<size_t, 3> QuadAssignmentTree<DataType>::getDataDims() const
    {
        return data_dims;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t QuadAssignmentTree<DataType>::getDataElementLen() const
    {
        return data_dims[0] * data_dims[1] * data_dims[2];
    }

    /**
     * @tparam VectorType
     * @return
     */
    template<typename VectorType>
    ParentType QuadAssignmentTree<VectorType>::getParentType() const
    {
        return parent_type;
    }
}

#endif //IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
