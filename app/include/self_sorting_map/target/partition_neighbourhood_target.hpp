#ifndef LDG_CORE_PARTITION_NEIGHBOURHOOD_TARGET_HPP
#define LDG_CORE_PARTITION_NEIGHBOURHOOD_TARGET_HPP

#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"

namespace ssm
{
    size_t PARTITION_NUM_BLOCKS_PER_DIMENSION = 4;

    /**
     * Load the neighbourhood targets into a target array.
     * The partition neighbourhood target basically aggregates the aggregates in the neighbourhood of th partition at the partition height.
     * This is very much just equivalent to convolution with an equally weighted NUM_BLOCKS_PER_DIMENSIONxNUM_BLOCKS_PER_DIMENSION kernel, just ignoring nullptrs.
     *
     * @tparam VectorType
     * @param target_map
     * @param quad_tree
     * @param distance_function
     * @param partition_height
     * @param is_shift
     */
    template<typename VectorType>
    void loadPartitionNeighbourhoodTargets(
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        const size_t partition_height,
        bool is_shift
    )
    {
        using namespace ldg;
        auto projected_dims = quad_tree.getBounds(partition_height).second;
        size_t num_elems = projected_dims.first * projected_dims.second;

        auto [num_rows, num_cols] = quad_tree.getBounds(0).second;
        size_t partition_len = size_t(std::pow(2, partition_height));
        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(PARTITION_NUM_BLOCKS_PER_DIMENSION * PARTITION_NUM_BLOCKS_PER_DIMENSION);

        int shift = is_shift ? 0 : (PARTITION_NUM_BLOCKS_PER_DIMENSION - 1) % 2;
        int blocks_offset = (PARTITION_NUM_BLOCKS_PER_DIMENSION - 1) / 2;

#pragma omp parallel for private(values) schedule(static)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            int partition_x = idx % projected_dims.second;
            int partition_y = idx / projected_dims.second;

            size_t min_y = std::max(partition_y - blocks_offset - (partition_y % 2 == 0 ? 1 : 0) * shift, 0);
            size_t max_y = std::min(static_cast<size_t>(partition_y + blocks_offset + (partition_y % 2) * shift), projected_dims.first);
            size_t min_x = std::max(partition_x - blocks_offset - (partition_x % 2 == 0 ? 1 : 0) * shift, 0);
            size_t max_x = std::min(static_cast<size_t>(partition_x + blocks_offset + (partition_x % 2) * shift), projected_dims.second);

            for (size_t y = min_y; y < max_y; ++y) {
                for (size_t x = min_x; x < max_x; ++x) {
                    values.push_back(quad_tree.getValue(CellPosition{ partition_height, rowMajorIndex(y, x, projected_dims.second) }));
                }
            }
            auto target = std::make_shared<VectorType>(quad_tree.getParentType() == ParentType::NORMALIZED_AVERAGE ?
                aggregate(values, quad_tree.getDataElementLen()) :
                findMinimum(values, distance_function)
            );

            // Copy to all relevant cells
            min_y = partition_y * partition_len;
            max_y = std::min(min_y + partition_len, num_rows);
            min_x = partition_x * partition_len;
            max_x = std::min(min_x + partition_len, num_cols);
            for (size_t y = min_y; y < max_y; ++y) {
                for (size_t x = min_x; x < max_x; ++x) {
                    target_map[rowMajorIndex(y, x, num_cols)].push_back(target);
                }
            }

            values.clear();
        }
    }
}

#endif //LDG_CORE_PARTITION_NEIGHBOURHOOD_TARGET_HPP
