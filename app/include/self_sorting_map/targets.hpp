#ifndef LDG_CORE_TARGETS_HPP
#define LDG_CORE_TARGETS_HPP

#include <vector>
#include <memory>
#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/target/hierarchy_target.hpp"
#include "app/include/self_sorting_map/target/neighbourhood_target.hpp"

namespace ssm
{

    /**
     * Get the sorting targets for a set of nodes with the algorithm at a certain height.
     * This function handles all loading of targets.
     *
     * @tparam VectorType
     * @param target_type
     * @param nodes
     * @param quad_tree
     * @param partition_height
     * @param is_shift
     * @return  A tuple of the number targets per node and the row-major num_nodes x num_targets vector.
     */
    template<typename VectorType>
    std::pair<size_t, std::vector<std::shared_ptr<VectorType>>> getTargets(
        TargetType target_type,
        std::vector<shared::CellPosition> &nodes,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        bool is_shift
    )
    {
        size_t num_targets = 0;
        size_t num_nodes = nodes.size();

        // Get the number of targets based on the target type.
        switch (target_type) {
            case TargetType::HIERARCHY: {
                auto height_bounds = getHierarchyTargetHeightBounds(quad_tree, partition_height, is_shift);
                num_targets += 1;

                std::vector<std::shared_ptr<VectorType>> targets(num_targets * num_nodes);
                loadHierarchyTargets(targets, 0, num_targets, height_bounds, nodes, quad_tree);
                return { num_targets, targets };
            }
            case TargetType::NEIGHBOURHOOD: {
                num_targets += 1;
                std::vector<std::shared_ptr<VectorType>> targets(num_targets * num_nodes);
                loadNeighbourhoodTargets(targets, 0, num_targets, partition_height, nodes, quad_tree);
                return { num_targets, targets };
            }
            case TargetType::HIERARCHY_NEIGHBOURHOOD: {
                auto height_bounds = getHierarchyTargetHeightBounds(quad_tree, partition_height, is_shift);
                num_targets += 2;

                std::vector<std::shared_ptr<VectorType>> targets(num_targets * num_nodes);
                loadHierarchyTargets(targets, 0, num_targets, height_bounds, nodes, quad_tree);
                loadNeighbourhoodTargets(targets, 1, num_targets, partition_height, nodes, quad_tree);
                return { num_targets, targets };
            }
        }
        return { 0, std::vector<std::shared_ptr<VectorType>>() };
    }
}


#endif //LDG_CORE_TARGETS_HPP
