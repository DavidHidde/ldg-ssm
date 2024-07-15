#ifndef LDG_CORE_TREE_FUNCTIONS_HPP
#define LDG_CORE_TREE_FUNCTIONS_HPP

#include <random>
#include <algorithm>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"
#include "app/include/ldg/util/math.hpp"
#include "app/include/program/random.hpp"

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
            assert(idx == assignment_copy[idx] && "Failed asserting the completeness of the assignment!");
        }
    }

    /**
     * Compute the parent of the quad tree based on the parent type.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     */
    template<typename VectorType>
    void computeParents(
        QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    ) {
        for (size_t height = 1; height < quad_tree.getDepth(); ++height) {
            auto [num_rows, num_cols] = quad_tree.getBounds(height).second;

#pragma omp parallel for schedule(static)
            for (size_t idx = 0; idx < num_rows * num_cols; ++idx) {
                CellPosition position{ height, idx };
                TreeWalker<VectorType> walker(position, num_rows, num_cols, quad_tree);
                auto children = walker.getChildrenValues();
                if (children[0] == nullptr && children[1] == nullptr && children[2] == nullptr && children[3] == nullptr) {
                    quad_tree.setValue(position, nullptr);
                } else {
                    std::vector<std::shared_ptr<VectorType>> child_vector(children.begin(), children.end());

                    VectorType parent_value = quad_tree.getParentType() == ParentType::NORMALIZED_AVERAGE ?
                        aggregate(child_vector, quad_tree.getDataElementLen()) :
                        findMinimum(child_vector, distance_function);

                    quad_tree.setValue(position, &parent_value);
                }
            }
        }
    }

    /**
     * Randomize a given assignment.
     * @param quad_tree
     *
     * @tparam VectorType
     */
    template<typename VectorType>
    void randomizeAssignment(QuadAssignmentTree<VectorType> &quad_tree)
    {
        auto &assignment = quad_tree.getAssignment();
        std::shuffle(assignment.begin(), assignment.begin() + quad_tree.getNumRows() * quad_tree.getNumCols(), program::RANDOMIZER);
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
