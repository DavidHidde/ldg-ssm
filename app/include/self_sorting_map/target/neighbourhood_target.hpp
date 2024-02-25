#ifndef LDG_CORE_NEIGHBOURHOOD_TARGET_HPP
#define LDG_CORE_NEIGHBOURHOOD_TARGET_HPP

#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Calculate the neighbourhood target by averaging a block of target vectors.
     *
     * @tparam VectorType
     * @param node
     * @param partition_height
     * @param quad_tree
     * @return
     */
    template<typename VectorType>
    std::shared_ptr<VectorType> getNeighbourHoodTarget(
        shared::CellPosition &node,
        size_t partition_height,
        shared::QuadAssignmentTree<VectorType> &quad_tree
    )
    {
        using namespace shared;
        VectorType target;
        TreeWalker<VectorType> walker(node, quad_tree);

        while (walker.moveUp() && walker.getNode().height < partition_height) {
            continue; // We need to iterate the walker up until we are at the parent.
        }
        auto parent_value = walker.getNodeValue();
        auto height_dims = quad_tree.getBounds(walker.getNode()).second;
        size_t parent_x = walker.getNode().index % height_dims.second;
        size_t parent_y = walker.getNode().index / height_dims.second;

        size_t count = 0;
        for (size_t y = std::max(0, int(parent_y) - 1); y < std::min(height_dims.first, parent_y + 1); ++y) {
            for (size_t x = std::max(0, int(parent_x) - 1); x < std::min(height_dims.first, parent_x + 1); ++x) {
                auto value_ptr = quad_tree.getValue(CellPosition{ walker.getNode().height, rowMajorIndex(y, x, height_dims.second) });
                if (value_ptr != nullptr) {
                    target += *value_ptr;
                    ++count;
                }
            }
        }

        return std::make_shared<VectorType>(target / double(count));
    }

    /**
     * Load the neighbourhood targets for the current nodes.
     *
     * @tparam VectorType
     * @param targets
     * @param col_offset
     * @param num_targets
     * @param partition_height
     * @param nodes
     * @param quad_tree
     */
    template<typename VectorType>
    void loadNeighbourhoodTargets(
        std::vector<std::shared_ptr<VectorType>> &targets,
        size_t col_offset,
        size_t num_targets,
        size_t partition_height,
        std::vector<shared::CellPosition> &nodes,
        shared::QuadAssignmentTree<VectorType> &quad_tree
    )
    {
        size_t num_nodes = nodes.size();
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            targets[shared::rowMajorIndex(idx, col_offset, num_targets)] = getNeighbourHoodTarget(nodes[idx], partition_height, quad_tree);
        }
    }
}

#endif //LDG_CORE_NEIGHBOURHOOD_TARGET_HPP
