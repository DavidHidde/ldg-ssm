#ifndef LDG_CORE_TARGETS_HPP
#define LDG_CORE_TARGETS_HPP

#include <vector>
#include <memory>
#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/target/partition_neighbourhood_target.hpp"
#include "app/include/self_sorting_map/target/highest_parent_hierarchy.hpp"

namespace ssm
{

    /**
     * Calculate the targets per node for a given target type.
     * We calculate everything at once to be able to efficiently reuse targets.
     *
     * @tparam VectorType
     * @param target_type
     * @param quad_tree
     * @param partition_height
     * @param is_shift
     * @return  A num_rows x num_cols map at the comparison height where each index contains the targets for that node.
     */
    template<typename VectorType>
    std::vector<std::vector<std::shared_ptr<VectorType>>> getTargetMap(
        const TargetType target_type,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        bool is_shift
    )
    {
        auto [num_rows, num_cols] = quad_tree.getBounds(0).second;
        std::vector<std::vector<std::shared_ptr<VectorType>>> target_map(num_rows * num_cols);

        switch (target_type) {
            case HIGHEST_PARENT_HIERARCHY:
                loadHighestParentHierarchyTargets(target_map, quad_tree, partition_height, is_shift);
            case PARTITION_NEIGHBOURHOOD:
                loadPartitionNeighbourhoodTargets(target_map, quad_tree, partition_height, is_shift);
                break;
        }

        return target_map;
    }
}


#endif //LDG_CORE_TARGETS_HPP
