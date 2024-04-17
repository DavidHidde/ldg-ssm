#ifndef LDG_CORE_TREE_FUNCTIONS_HPP
#define LDG_CORE_TREE_FUNCTIONS_HPP

#include <random>
#include <algorithm>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"
#include "app/include/ldg/util/math.hpp"

#include <cassert>

namespace ldg
{
    /**
     * Assert that the elements in the grid only appear once.
     *
     * @tparam VectorType
     * @param quad_tree
     */
    template<typename VectorType>
    void assertUniqueAssignment(QuadAssignmentTree<VectorType> &quad_tree)
    {
        auto assignment_copy(quad_tree.getAssignment());
        std::sort(assignment_copy.begin(), assignment_copy.end());
        for (size_t idx = 0; idx < assignment_copy.size(); ++idx) {
            auto copy = assignment_copy[idx];
            assert(idx == assignment_copy[idx] && "Failed asserting the completeness of the assignment!");
        }
    }

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
#pragma omp parallel for schedule(static)
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
    void randomizeAssignment(QuadAssignmentTree<VectorType> &quad_tree, size_t seed)
    {
        auto &assignment = quad_tree.getAssignment();
        std::shuffle(assignment.begin(), assignment.begin() + quad_tree.getNumRows() * quad_tree.getNumCols(), std::mt19937(seed));
    }

    /**
     * Create a direct assignment for given dimensions.
     *
     * @param size
     * @return
     */
    inline std::vector<size_t> createAssignment(const size_t size)
    {
        auto assignment = std::vector<size_t>(size);
        std::iota(assignment.begin(), assignment.end(), 0);
        return assignment;
    }

    /**
     * Determine the size of the array required to fit a num_rows x num_cols quad tree
     *
     * @param num_rows
     * @param num_cols
     * @return
     */
    inline size_t determineRequiredArrayCapacity(size_t num_rows, size_t num_cols)
    {
        size_t num_elements = num_rows * num_cols;
        size_t size = num_elements;
        size_t new_num_cols = num_cols;
        size_t new_num_rows = num_rows;
        while (new_num_rows > 1 || new_num_cols > 1) {
            new_num_cols = ceilDivideByFactor(new_num_cols, 2.);
            new_num_rows = ceilDivideByFactor(new_num_rows, 2.);
            size += new_num_cols * new_num_rows;
        }

        return size;
    }
}

#endif //LDG_CORE_TREE_FUNCTIONS_HPP
