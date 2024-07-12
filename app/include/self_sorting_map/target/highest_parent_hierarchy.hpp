#ifndef LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP
#define LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP

#include <vector>
#include <memory>
#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/ldg/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Load the parent targets into a targets data array.
     * This target is the last unique parent when considering comparisons, representing a hierarchical neighbourhood.
     *
     * @tparam VectorType
     * @param target_map
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     */
    template<typename VectorType>
    void loadHighestParentHierarchyTargets(
        std::vector<std::shared_ptr<VectorType>> &target_map,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        const size_t partition_height,
        const size_t comparison_height,
        bool is_shift
    )
    {
        using namespace ldg;
        auto projected_dims = quad_tree.getBounds(partition_height).second;
        auto comparison_height_dims = quad_tree.getBounds(comparison_height).second;
        size_t num_elems = projected_dims.first * projected_dims.second;
        size_t partition_len = size_t(std::pow(2, partition_height - comparison_height));

#pragma omp parallel for schedule(static)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            int partition_x = idx % projected_dims.second;
            int partition_y = idx / projected_dims.second;
            size_t max_parent_height = is_shift ? partition_height + 1: partition_height;

            TreeWalker<VectorType> walker{ CellPosition{ partition_height, idx }, quad_tree };
            for (size_t height = partition_height; height < max_parent_height; ++height) {
                walker.moveUp();
            }
            auto target = walker.getNodeValue();

            // Copy to all relevant cells
            size_t min_y = partition_y * partition_len;
            size_t max_y = std::min(min_y + partition_len, comparison_height_dims.first);
            size_t min_x = partition_x * partition_len;
            size_t max_x = std::min(min_x + partition_len, comparison_height_dims.second);
            for (size_t y = min_y; y < max_y; ++y) {
                for (size_t x = min_x; x < max_x; ++x) {
                    target_map[rowMajorIndex(y, x, comparison_height_dims.second)] = target;
                }
            }
        }
    }
}

#endif //LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP
