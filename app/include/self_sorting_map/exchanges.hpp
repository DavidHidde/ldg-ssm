#ifndef LDG_CORE_EXCHANGES_HPP
#define LDG_CORE_EXCHANGES_HPP

#include <functional>
#include "app/include/ldg/model/quad_assignment_tree.hpp"

namespace ssm
{
    /**
     * Compare nodes and find the permutation which minimizes the distance to all targets.
     * Afterwards, exchange all items into this permutation.
     *
     * @tparam VectorType
     * @param nodes
     * @param quad_tree
     * @param distance_function
     * @param target_map
     * @return The number of exchanges performed.
    */
    template<typename VectorType>
    size_t findAndSwapBestPermutation(
        std::vector<ldg::CellPosition> &nodes,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::vector<std::shared_ptr<VectorType>> &target_map
    )
    {
        using namespace ldg;
        // Precheck if all partitions are of the same size, since this is a requirement for swapping.
        size_t num_nodes = nodes.size();
        size_t num_leaves = 0;
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            auto dims = quad_tree.getLeafBounds(nodes[idx]).second;
            size_t node_num_leaves = dims.first * dims.second;
            if (num_leaves == 0) {
                num_leaves = node_num_leaves;
            } else if (node_num_leaves != num_leaves) {
                return 0;
            }
        }

        // Preload data and assignments
        std::vector<std::vector<size_t>> node_assignments(num_nodes);   // Assigned indices per node
        std::vector<std::shared_ptr<VectorType>> node_data(num_nodes);  // Actual data per node
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            node_data[idx] = quad_tree.getValue(nodes[idx]);
            auto leaf_iterator = (TreeWalker<VectorType>(nodes[idx], quad_tree)).getLeaves();
            node_assignments[idx].reserve(num_leaves);
            for (; leaf_iterator != leaf_iterator.end(); ++leaf_iterator)
                node_assignments[idx].push_back(quad_tree.getAssignmentValue(leaf_iterator.getPosition()));
        }

        // Find best permutation
        double minimum_distance = -1.; // We assume the distance to always be positive, so this flags this to uninitialized.
        std::vector<size_t> best_permutation(num_nodes);
        for (size_t idx = 0; idx < num_nodes; ++idx)
            best_permutation[idx] = idx;
        std::vector permutation(best_permutation);

        do {
            double distance = 0.;
            for (size_t idx = 0; idx < num_nodes; ++idx) {
                distance += distance_function(
                    node_data[permutation[idx]],
                    target_map[nodes[idx].index]
                );
            }

            // Better permutation, save it
            if (minimum_distance == -1 || minimum_distance > distance) {
                minimum_distance = distance;
                best_permutation = permutation;
            }
        } while (std::next_permutation(permutation.begin(), permutation.end()));

        // Perform the actual exchanges where needed
        size_t exchanges_count = 0;
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            if (best_permutation[idx] != idx) {
                auto leaf_iterator = (TreeWalker<VectorType>(nodes[idx], quad_tree)).getLeaves();
                for (size_t leaf_idx = 0; leaf_iterator != leaf_iterator.end(); ++leaf_iterator, ++leaf_idx)
                    quad_tree.setAssignmentValue(leaf_iterator.getPosition(), node_assignments[best_permutation[idx]][leaf_idx]);
                ++exchanges_count;
            }
        }

        return exchanges_count;
    }
}

#endif //LDG_CORE_EXCHANGES_HPP
