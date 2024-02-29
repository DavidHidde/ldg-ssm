#ifndef LDG_CORE_NEIGHBOURHOOD_TARGET_HPP
#define LDG_CORE_NEIGHBOURHOOD_TARGET_HPP

#include "app/include/shared/model/cell_position.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace ssm
{
    size_t NUM_BLOCKS_PER_DIMENSION = 4;

    /**
     * Load the neighbourhood targets into a target array.
     * The neighbourhood target basically aggregates the aggregates in the neighbourhood of th partition at the partition height.
     * This is very much just equivalent to convolution with an equally weighted NUM_BLOCKS_PER_DIMENSIONxNUM_BLOCKS_PER_DIMENSION kernel, just ignoring nullptrs.
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

        int shift = is_shift ? 0 : (NUM_BLOCKS_PER_DIMENSION - 1) % 2;
        int blocks_offset = (NUM_BLOCKS_PER_DIMENSION - 1) / 2;

        // Aggregate parent targets and assign them to all relevant cells at the comparison height.
#pragma omp parallel for private(values)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            int partition_x = idx % projected_dims.second;
            int partition_y = idx / projected_dims.second;

            size_t min_y = std::max(partition_y - blocks_offset - (partition_y % 2 == 0 ? 1 : 0) * shift, 0);
            size_t max_y = std::min(size_t(partition_y + blocks_offset + (partition_y % 2) * shift), projected_dims.first - 1);
            size_t min_x = std::max(partition_x - blocks_offset - (partition_x % 2 == 0 ? 1 : 0) * shift, 0);
            size_t max_x = std::min(size_t(partition_x + blocks_offset + (partition_x % 2) * shift), projected_dims.second - 1);

            for (size_t y = min_y; y <= max_y; ++y) {
                for (size_t x = min_x; x <= max_x; ++x) {
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
