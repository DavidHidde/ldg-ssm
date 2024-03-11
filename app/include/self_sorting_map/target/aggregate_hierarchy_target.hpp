#ifndef LDG_CORE_AGGREGATE_HIERARCHY_TARGET_HPP
#define LDG_CORE_AGGREGATE_HIERARCHY_TARGET_HPP

#include <vector>
#include <memory>
#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Get the minimum and maximum height for calculating the hierarchy targets.
     *
     * @param partition_height
     * @param is_shift
     * @return
     */
    inline std::pair<size_t, size_t> getHierarchyTargetHeightBounds(
        size_t partition_height,
        const bool is_shift
    )
    {
        return {
            partition_height,
            is_shift ? partition_height + 2 : partition_height + 1
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
    void loadAggregateHierarchyTargets(
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        const size_t partition_height,
        const size_t comparison_height,
        const bool is_shift
    )
    {
        using namespace ldg;
        auto [min_height, max_height] = getHierarchyTargetHeightBounds(partition_height, is_shift);
        size_t num_parents = max_height - min_height;

        // We iterate over the dims of the first parent, since this marks the partition with the same parents and therefore targets.
        auto projected_dims = quad_tree.getBounds(CellPosition{ min_height, 0 }).second;
        size_t num_elems = projected_dims.first * projected_dims.second;

        auto [num_rows, num_cols] = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        auto partition_len = static_cast<size_t>(std::pow(2, min_height - comparison_height));
        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(num_parents);

#pragma omp parallel for private(values)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            TreeWalker<VectorType> walker{ CellPosition{ min_height, idx }, quad_tree };
            for (size_t parent_idx = 0; parent_idx < num_parents; ++parent_idx) {
                values.push_back(walker.getNodeValue());
                walker.moveUp();
            }
            auto target = std::make_shared<VectorType>(aggregate(values, quad_tree.getDataElementLen()));

            // Copy to all relevant cells
            size_t partition_x = idx % projected_dims.second;
            size_t partition_y = idx / projected_dims.second;
            size_t min_y = partition_y * partition_len;
            size_t max_y = std::min(min_y + partition_len, num_rows);
            size_t min_x = partition_x * partition_len;
            size_t max_x = std::min(min_x + partition_len, num_cols);
            for (size_t y = min_y; y < max_y; ++y) {
                for (size_t x = min_x; x < max_x; ++x) {
                    target_map[rowMajorIndex(y, x, num_cols)].push_back(target);
                }
            }

            values.clear();
        }
    }
}

#endif //LDG_CORE_AGGREGATE_HIERARCHY_TARGET_HPP
