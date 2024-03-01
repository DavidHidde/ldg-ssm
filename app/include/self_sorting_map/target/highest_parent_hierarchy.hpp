#ifndef LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP
#define LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP

#include <vector>
#include <memory>
#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Get a buffer for the longest dimension axis to determine the number of parents to traverse before the parents become the same.
     * This buffer can be used to determine the last unique parent when comparing nodes in even-odd or odd-even configurations.
     *
     * For an even-odd iteration, this has to be partition_height due to the quad_tree structure, but for an even-odd iteration
     * every node breaches into different partitions and therefore this becomes a function of the axis length.
     *
     * @tparam VectorType
     * @param partition_height
     * @param quad_tree
     * @param is_shift
     * @return
     */
    template<typename VectorType>
    std::vector<size_t> getMaxHeightBuffer(size_t partition_height, shared::QuadAssignmentTree<VectorType> &quad_tree, bool is_shift)
    {
        auto partition_dims = quad_tree.getBounds(shared::CellPosition{ partition_height, 0 }).second;
        std::size_t size = std::max(partition_dims.first, partition_dims.second);

        if (!is_shift) {
            return std::vector<size_t>(size, partition_height);
        }

        std::vector<size_t> buffer;
        buffer.reserve(size);
        size_t max_height = partition_height + 1;
        size_t base = 4;
        size_t max_value = size_t(std::pow(2., std::ceil(std::log2(size)) - 1));
        // Basically count how many hierarchy levels we've passed. This needs to be inversed after the middle since we also should count from the bottom
        for (size_t idx = 0; idx < size; ++idx) {
            if (idx < max_value) {  // Top-down hierarchy size counting
                if (idx >= base) {
                    base *= 2;
                    max_height += 1;
                }
                buffer.push_back(max_height);
            } else { // Bottom-up hierarchy size counting
                buffer.push_back(buffer[2 * max_value - idx - 1]);
            }
        }

        return buffer;
    }

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
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        using namespace shared;
        auto projected_dims = quad_tree.getBounds(CellPosition{ partition_height, 0 }).second;
        auto comparison_height_dims = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        size_t num_elems = projected_dims.first * projected_dims.second;
        size_t partition_len = size_t(std::pow(2, partition_height - comparison_height));
        std::vector<size_t> max_height_buffer = getMaxHeightBuffer(partition_height, quad_tree, is_shift);

//#pragma omp parallel for
        for (size_t idx = 0; idx < num_elems; ++idx) {
            size_t partition_x = idx % projected_dims.second;
            size_t partition_y = idx / projected_dims.second;
            size_t max_parent_height = max_height_buffer[std::max(partition_x, partition_y)];

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
                    target_map[rowMajorIndex(y, x, comparison_height_dims.second)].push_back(target);
                }
            }
        }
    }
}

#endif //LDG_CORE_HIGHEST_PARENT_HIERARCHY_HPP
