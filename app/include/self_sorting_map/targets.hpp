#ifndef LDG_CORE_TARGETS_HPP
#define LDG_CORE_TARGETS_HPP

#include <vector>
#include <memory>
#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/target/aggregate_hierarchy_target.hpp"
#include "app/include/self_sorting_map/target/partition_neighbourhood_target.hpp"
#include "app/include/self_sorting_map/target/cell_neighbourhood_target.hpp"
#include "app/include/self_sorting_map/target/highest_parent_hierarchy.hpp"
#include "app/include/self_sorting_map/target/4_connected_target.hpp"

namespace ssm
{

    /**
     * Calculate all targets per node for a given target type.
     * We calculate everything at once to be able to efficiently reuse targets.
     *
     * @tparam VectorType
     * @param target_types
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     * @return  A num_rows x num_cols map at the comparison height where each index contains the targets for that node.
     */
    template<typename VectorType>
    std::vector<std::vector<std::shared_ptr<VectorType>>> getTargetMap(
        std::vector<TargetType> const &target_types,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        auto comparison_height_dims = quad_tree.getBounds(comparison_height).second;
        std::vector<std::vector<std::shared_ptr<VectorType>>> target_map(comparison_height_dims.first * comparison_height_dims.second);

        for (TargetType target_type: target_types) {
            switch (target_type) {
                case AGGREGATE_HIERARCHY:
                    loadAggregateHierarchyTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);
                    break;
                case HIGHEST_PARENT_HIERARCHY:
                    loadHighestParentHierarchyTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);
                case PARTITION_NEIGHBOURHOOD:
                    loadPartitionNeighbourhoodTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);
                    break;
                case CELL_NEIGHBOURHOOD:
                    loadCellNeighbourhoodTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);
                    break;
                case AGGREGATE_HIERARCHY_4C:
                    load4ConnectedTarget(
                        loadAggregateHierarchyTargets<VectorType>,
                        target_map,
                        quad_tree,
                        partition_height,
                        comparison_height,
                        is_shift
                    );
                    break;
                case HIGHEST_PARENT_HIERARCHY_4C:
                    load4ConnectedTarget(
                        loadHighestParentHierarchyTargets<VectorType>,
                        target_map,
                        quad_tree,
                        partition_height,
                        comparison_height,
                        is_shift
                    );
                    break;
            }
        }

        return target_map;
    }
}


#endif //LDG_CORE_TARGETS_HPP
