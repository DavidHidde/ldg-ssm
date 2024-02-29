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
     * Calculate all targets per node for a given target type.
     * We calculate everything at once to be able to efficiently reuse targets.
     *
     * @tparam VectorType
     * @param target_type
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     * @return  A num_rows x num_cols map at the comparison height where each index contains the targets for that node.
     */
    template<typename VectorType>
    std::vector<std::vector<std::shared_ptr<VectorType>>> getTargetMap(
        TargetType target_type,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        auto comparison_height_dims = quad_tree.getBounds(shared::CellPosition{ comparison_height, 0 }).second;
        std::vector<std::vector<std::shared_ptr<VectorType>>> target_map(comparison_height_dims.first * comparison_height_dims.second);

        if (target_type == TargetType::HIERARCHY || target_type == TargetType::HIERARCHY_NEIGHBOURHOOD)
            loadHierarchyTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);
        if (target_type == TargetType::NEIGHBOURHOOD || target_type == TargetType::HIERARCHY_NEIGHBOURHOOD)
            loadNeighbourhoodTargets(target_map, quad_tree, partition_height, comparison_height, is_shift);

        return target_map;
    }
}


#endif //LDG_CORE_TARGETS_HPP
