#ifndef LDG_CORE_NEIGHBOURHOOD_TARGET_HPP
#define LDG_CORE_NEIGHBOURHOOD_TARGET_HPP

#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    /**
     * Load the neighbourhood targets into a target array.
     * The neighbourhood target basically aggregates the aggregates in the neighbourhood of th partition at the partition height.
     * This is very much just equivalent to convolution with an equally weighted 3x3 kernel, just ignoring nullptrs.
     *
     * @tparam VectorType
     * @param target_map
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     */
    template<typename VectorType>
    void loadNeighbourhoodTargets(
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        using namespace shared;
        auto projected_dims = quad_tree.getBounds(CellPosition{ partition_height, 0 }).second;
        size_t num_elems = projected_dims.first * projected_dims.second;

        auto comparison_height_dims = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        size_t partition_len = size_t(std::pow(2, partition_height - comparison_height));
        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(9);

        // Aggregate parent targets and assign them to all relevant cells at the comparison height.
#pragma omp parallel for private(values)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            size_t partition_x = idx % projected_dims.second;
            size_t partition_y = idx / projected_dims.second;
            for (size_t y = std::max(0, int(partition_y) - 1); y < std::min(projected_dims.first, partition_y + 1); ++y) {
                for (size_t x = std::max(0, int(partition_x) - 1); x < std::min(projected_dims.first, partition_x + 1); ++x) {
                    values.push_back(quad_tree.getValue(CellPosition{ partition_height, rowMajorIndex(y, x, projected_dims.second) }));
                }
            }
            auto target = std::make_shared<VectorType>(aggregate(values));

            // Copy to all relevant cells
            for (size_t y = 0; y < partition_len; ++y) {
                for (size_t x = 0; x < partition_len; ++x) {
                    target_map[rowMajorIndex(partition_y * partition_len + y, partition_x * partition_len + x, comparison_height_dims.second)].push_back(target);
                }
            }

            values.clear();
        }
    }
}

#endif //LDG_CORE_NEIGHBOURHOOD_TARGET_HPP
