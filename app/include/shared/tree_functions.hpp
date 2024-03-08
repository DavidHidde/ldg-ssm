#ifndef LDG_CORE_TREE_FUNCTIONS_HPP
#define LDG_CORE_TREE_FUNCTIONS_HPP

#include <random>
#include <algorithm>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"
#include "app/include/shared/util/vector_math.hpp"

namespace shared
{
    /**
     * Compute and update the aggregates of the quad tree.
     * For this we assume we can perform basic arithmetic functions on VectorType.
     *
     * @tparam VectorType
     */
    template<typename VectorType>
    void computeAggregates(QuadAssignmentTree<VectorType> &quad_tree)
    {
        size_t curr_num_rows = ceilDivideByFactor(quad_tree.getNumRows(), 2.);
        size_t curr_num_cols = ceilDivideByFactor(quad_tree.getNumCols(), 2.);

        // Compute the aggregates bottom-up
        for (size_t height = 1; height < quad_tree.getDepth(); ++height) {
#pragma omp parallel for
            for (size_t idx = 0; idx < curr_num_rows * curr_num_cols; ++idx) {
                CellPosition position{ height, idx };
                TreeWalker<VectorType> walker(position, curr_num_rows, curr_num_cols, quad_tree);
                auto children = walker.getChildrenValues();
                VectorType aggregated_value = aggregate(std::vector<std::shared_ptr<VectorType>>(children.begin(), children.end()), quad_tree.getDataElementLen());
                quad_tree.setValue(position, aggregated_value);
            }

            curr_num_rows = ceilDivideByFactor(curr_num_rows, 2.);
            curr_num_cols = ceilDivideByFactor(curr_num_cols, 2.);
        }
    }

    /**
     * Randomize a given assignment.
     * @param quad_tree
     *
     * @tparam VectorType
     */
    template<typename VectorType>
    void randomizeAssignment(shared::QuadAssignmentTree<VectorType> &quad_tree, size_t seed)
    {
        auto &assignment = quad_tree.getAssignment();
        std::shuffle(assignment.begin(), assignment.begin() + quad_tree.getNumRows() * quad_tree.getNumCols(), std::mt19937(seed));
    }

    /**
     * Create a direct assignment for given dimensions.
     *
     * @param num_rows
     * @param num_cols
     * @return
     */
    std::vector<size_t> createAssignment(size_t size, size_t num_rows, size_t num_cols)
    {
        auto assignment = std::vector<size_t>(size);

        // Generate quad tree structure space, where every height starts at 0 again
        size_t offset = 0;
        while (num_rows > 1 && num_cols > 1) {
            size_t end = offset + num_cols * num_rows;
            for (size_t idx = offset; idx < end; ++idx) {
                assignment[idx] = idx;
            }

            offset += num_cols * num_rows;
            num_cols = shared::ceilDivideByFactor(num_cols, 2.);
            num_rows = shared::ceilDivideByFactor(num_rows, 2.);
        }
        assignment[size - 1] = size - 1; // Fix last element

        return assignment;
    }

    /**
     * Determine the size of the array required to fit a num_rows x num_cols quad tree
     *
     * @param num_rows
     * @param num_cols
     * @return
     */
    size_t determineRequiredArrayCapacity(size_t num_rows, size_t num_cols)
    {
        size_t num_elements = num_rows * num_cols;
        size_t size = num_elements;
        size_t new_num_cols = num_cols;
        size_t new_num_rows = num_rows;
        while (new_num_rows > 1 && new_num_cols > 1) {
            new_num_cols = shared::ceilDivideByFactor(new_num_cols, 2.);
            new_num_rows = shared::ceilDivideByFactor(new_num_rows, 2.);
            size += new_num_cols * new_num_rows;
        }

        return size;
    }
}

#endif //LDG_CORE_TREE_FUNCTIONS_HPP
