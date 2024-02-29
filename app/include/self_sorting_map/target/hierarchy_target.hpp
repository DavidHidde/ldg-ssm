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
     * This basically aggregates parents from a minimum height to a maximum height as determined by getHierarchyTargetHeightBounds
     *
     * @tparam VectorType
     * @param target_map
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     */
    template<typename VectorType>
    void loadHierarchyTargets(
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        using namespace shared;
        auto height_bounds = getHierarchyTargetHeightBounds(quad_tree, partition_height, is_shift);
        size_t num_parents = height_bounds.second - height_bounds.first;

        // We iterate over the dims of the first parent, since this marks the partition with the same parents and therefore targets.
        auto projected_dims = quad_tree.getBounds(CellPosition{ height_bounds.first, 0 }).second;
        size_t num_elems = projected_dims.first * projected_dims.second;

        auto comparison_height_dims = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        size_t partition_len = size_t(std::pow(2, height_bounds.first - comparison_height));
        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(num_parents);

        // Aggregate parent targets and assign them to all relevant cells at the comparison height.
#pragma omp parallel for private(values)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            TreeWalker<VectorType> walker{ CellPosition{ height_bounds.first, idx }, quad_tree };
            for (size_t parent_idx = 0; parent_idx < num_parents; ++parent_idx) {
                values.push_back(walker.getNodeValue());
                walker.moveUp();
            }
            auto target = std::make_shared<VectorType>(aggregate(values));

            // Copy to all relevant cells
            size_t partition_x = idx % projected_dims.second;
            size_t partition_y = idx / projected_dims.second;
            for (size_t y = 0; y < partition_len; ++y) {
                for (size_t x = 0; x < partition_len; ++x) {
                    target_map[rowMajorIndex(partition_y * partition_len + y, partition_x * partition_len + x, comparison_height_dims.second)].push_back(target);
                }
            }

            values.clear();
        }
    }
}

#endif //LDG_CORE_HIERARCHY_TARGET_HPP
