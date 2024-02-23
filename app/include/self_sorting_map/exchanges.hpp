#ifndef LDG_CORE_EXCHANGES_HPP
#define LDG_CORE_EXCHANGES_HPP

#include <functional>
#include "app/include/shared/model/quad_assignment_tree.hpp"

namespace ssm
{
    /**
     * Compare nodes and find the permutation which minimizes the distance to all parents.
     * Afterwards, swap all items into this permutation.
     *
     * @tparam VectorType
     * @param nodes
     * @param quad_tree
     * @param distance_function
     * @param num_targets
     * @param targets
     * @return The number of swaps performed.
    */
    template<typename VectorType>
    size_t findAndSwapBestPermutation(
        std::vector<shared::CellPosition> &nodes,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        size_t num_targets,
        std::vector<std::shared_ptr<VectorType>> &targets
    )
    {
        using namespace shared;
        // Load all the nodes and parents beforehand
        size_t num_nodes = nodes.size();
        size_t height_offset = quad_tree.getBounds(nodes[0]).first.first;

        std::vector<size_t> node_assignments(num_nodes);                                    // Assigned indices per node
        std::vector<std::shared_ptr<VectorType>> node_data(num_nodes);                      // Actual data per node

        for (size_t idx = 0; idx < num_nodes; ++idx) {
            node_data[idx] = quad_tree.getValue(nodes[idx]);
            node_assignments[idx] = quad_tree.getAssignment()[nodes[idx].index + height_offset];
        }

        // Find best permutation
        double minimum_distance = -1.; // We assume the distance to always be positive, so this flags this to uninitialized.
        std::vector<size_t> best_permutation(num_nodes);
        for (size_t idx = 0; idx < num_nodes; ++idx)
            best_permutation[idx] = idx;
        std::vector<size_t> permutation(best_permutation);

        do {
            double distance = 0.;
            for (size_t idx = 0; idx < num_nodes; ++idx) {
                for (size_t parent_idx = 0; parent_idx < num_targets; ++parent_idx) {
                    distance += distance_function(
                        node_data[permutation[idx]],
                        targets[rowMajorIndex(idx, parent_idx, num_targets)]
                    );
                }
            }

            // Better permutation, save it
            if (minimum_distance == -1 || minimum_distance > distance) {
                minimum_distance = distance;
                best_permutation = permutation;
            }
        } while (std::next_permutation(permutation.begin(), permutation.end()));

        // Perform the actual swapping where needed
        size_t swap_count = 0;
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            if (best_permutation[idx] != idx) {
                quad_tree.setAssignment(nodes[idx], node_assignments[best_permutation[idx]]);
                ++swap_count;
            }
        }

        return swap_count;
    }
}

#endif //LDG_CORE_EXCHANGES_HPP
