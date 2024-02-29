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
                VectorType aggregated_value = aggregate(std::vector<std::shared_ptr<VectorType>>(children.begin(), children.end()));
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
        std::shuffle(assignment.begin(), assignment.begin() + quad_tree.getNumRows() * quad_tree.getNumCols(), std::mt19937());
    }
}

#endif //LDG_CORE_TREE_FUNCTIONS_HPP
