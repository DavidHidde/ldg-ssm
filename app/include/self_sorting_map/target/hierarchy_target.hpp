#ifndef LDG_CORE_HIERARCHY_TARGET_HPP
#define LDG_CORE_HIERARCHY_TARGET_HPP

#include <vector>
#include <memory>
#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Get the minimum and maximum height for calculating the hierarchy targets.
     *
     * @tparam VectorType
     * @param nodes
     * @param quad_tree
     * @param partition_height
     * @param is_shift
     * @return
     */
    template<typename VectorType>
    std::pair<size_t, size_t> getHierarchyTargetHeightBounds(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        bool is_shift
    )
    {
        return {
            partition_height,
            is_shift ? quad_tree.getDepth() - 1 : partition_height + 1
        };
    }

    /**
     * Load the parent targets into a targets data array.
     *
     * @tparam VectorType
     * @param targets
     * @param col_offset
     * @param num_targets
     * @param height_bounds The bounds calculated by {@see getHierarchyTargetHeightBounds}
     * @param nodes
     * @param quad_tree
     */
    template<typename VectorType>
    void loadHierarchyTargets(
        std::vector<std::shared_ptr<VectorType>> &targets,
        size_t col_offset,
        size_t num_targets,
        std::pair<size_t, size_t> height_bounds,
        std::vector<shared::CellPosition> &nodes,
        shared::QuadAssignmentTree<VectorType> &quad_tree
    )
    {
        size_t min_height = height_bounds.first;
        size_t max_height = height_bounds.second;
        size_t num_nodes = nodes.size();

        for (size_t idx = 0; idx < num_nodes; ++idx) {
            shared::TreeWalker<VectorType> walker{ nodes[idx], quad_tree };
            for (size_t parent_idx = 1; parent_idx < max_height && walker.moveUp(); ++parent_idx) {
                if (parent_idx >= min_height) {
                    size_t new_idx = shared::rowMajorIndex(idx, parent_idx - min_height + col_offset, num_targets);
                    targets[new_idx] = walker.getNodeValue();
                }
            }
        }
    }
}

#endif //LDG_CORE_HIERARCHY_TARGET_HPP
