#ifndef NEW_LDG_DISPARITY_HPP
#define NEW_LDG_DISPARITY_HPP

#include <vector>
#include <functional>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"

namespace ldg
{
    template<typename VectorType>
    inline std::vector<double> computeDisparity(
        QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    )
    {
        size_t num_leafs = quad_tree.getNumRows() * quad_tree.getNumCols();
        size_t num_nodes = quad_tree.getAssignment().size();
        auto disparities = std::vector<double>(num_nodes, 0.);

        for (size_t idx = 0; idx < num_leafs; ++idx) {
            auto leaf_value = quad_tree.getValue({ 0, idx });
            TreeWalker walker{ { 0, idx }, quad_tree };
            while (walker.moveUp()) {
                disparities[quad_tree.getAssignmentValue(walker.getNode())] += distance_function(leaf_value, walker.getNodeValue());
            }
        }

        // Normalize
        double root_value = disparities[disparities.size() - 1];
#pragma omp parallel for schedule(static)
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            disparities[idx] /= root_value;
        }

        return disparities;
    }
}

#endif //NEW_LDG_DISPARITY_HPP
