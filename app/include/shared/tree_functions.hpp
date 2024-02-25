#ifndef LDG_CORE_TREE_FUNCTIONS_HPP
#define LDG_CORE_TREE_FUNCTIONS_HPP

#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

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
                float count = 0.;
                VectorType aggregate;
                for (std::shared_ptr<VectorType> child: walker.getChildrenValues()) {
                    if (child != nullptr) {
                        ++count;
                        aggregate += *child;
                    }
                }
                aggregate /= count;
                quad_tree.setValue(position, aggregate);
            }

            curr_num_rows = ceilDivideByFactor(curr_num_rows, 2.);
            curr_num_cols = ceilDivideByFactor(curr_num_cols, 2.);
        }
    }
}

#endif //LDG_CORE_TREE_FUNCTIONS_HPP
