#ifndef LDG_CORE_PARTITIONS_HPP
#define LDG_CORE_PARTITIONS_HPP

#include <functional>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/exchanges.hpp"

namespace ssm
{
    /**
     * Perform the exchanges of the self-sorting map. This functions handles pairing up the right data and then getting it compared.
     * This functions goes over the data without the use of the fancy iterators to allow easy element-wise comparisons for better
     * parallelizability.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param target_type
     * @param comparison_height
     * @param apply_shift
     * @param partition_len Length of the current partition.
     * @param offset    Offset [rows, columns] for the calculated indices. This is used to project back to actual array indices
     * @param iteration_dims    The dimensions to be iterated over.
     * @param comparison_height_dims The dimensions at the height of comparison.
     * @return
     */
    template<typename VectorType>
    size_t performPartitionExchanges(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        TargetType target_type,
        size_t partition_height,
        size_t comparison_height,
        bool apply_shift,
        long partition_len,
        std::pair<long, long> &offset,
        std::pair<long, long> &iteration_dims,
        std::pair<size_t, size_t> &comparison_height_dims
    )
    {
        using namespace shared;
        size_t num_exchanges = 0;
        std::vector<CellPosition> nodes;
        nodes.reserve(4);

        long comparison_num_rows = comparison_height_dims.first;
        long comparison_num_cols = comparison_height_dims.second;
        long iteration_num_rows = iteration_dims.first;
        long iteration_num_cols = iteration_dims.second;

        long projected_num_rows = iteration_num_rows / 2 + (iteration_num_rows % (2 * partition_len)) % partition_len;
        long projected_num_cols = iteration_num_cols / 2 + (iteration_num_cols % (2 * partition_len)) % partition_len;
        long num_elems = projected_num_rows * projected_num_cols;

#pragma omp parallel for private(nodes) reduction(+:num_exchanges)
        for (long idx = 0; idx < num_elems; ++idx) {
            long projected_x = idx % projected_num_cols;
            long projected_y = idx / projected_num_cols;

            long partition_x = projected_x / partition_len;
            long partition_y = projected_y / partition_len;
            long within_partition_x = projected_x % partition_len;
            long within_partition_y = projected_y % partition_len;

            long base_x = offset.second + within_partition_x + partition_x * partition_len * 2;
            long base_y = offset.first + within_partition_y + partition_y * partition_len * 2;

            // Add all nodes within range
            nodes.clear();
            long node_x = base_x;
            long node_y = base_y;
            if (node_x >= 0 && node_x < comparison_num_cols && node_y >= 0 && node_y < comparison_num_rows)
                nodes.push_back(CellPosition{ comparison_height, rowMajorIndex(node_y, node_x, comparison_num_cols) });

            node_x = base_x + partition_len;
            if (node_x >= 0 && node_x < comparison_num_cols && node_y >= 0 && node_y < comparison_num_rows)
                nodes.push_back(CellPosition{ comparison_height, rowMajorIndex(node_y, node_x, comparison_num_cols) });

            node_x = base_x;
            node_y = base_y + partition_len;
            if (node_x >= 0 && node_x < comparison_num_cols && node_y >= 0 && node_y < comparison_num_rows)
                nodes.push_back(CellPosition{ comparison_height, rowMajorIndex(node_y, node_x, comparison_num_cols) });

            node_x = base_x + partition_len;
            node_y = base_y + partition_len;
            if (node_x >= 0 && node_x < comparison_num_cols && node_y >= 0 && node_y < comparison_num_rows)
                nodes.push_back(CellPosition{ comparison_height, rowMajorIndex(node_y, node_x, comparison_num_cols) });

            // Now perform exchanges if we have nodes to compare
            if (nodes.size() > 1) {
                auto target_data = getTargets(target_type, nodes, quad_tree, partition_height, apply_shift);
                num_exchanges += findAndSwapBestPermutation(nodes, quad_tree, distance_function, target_data.first, target_data.second);
            }
        }

        return num_exchanges;
    }

    /**
     * Start partition optimization by performing exchanges. This function in particular applies the even-odd or odd-even swapping.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param target_type   Type for the target comparisons
     * @param partition_height  The height of the partitions being compared.
     * @param comparison_height The height at which the elements should be compared.
     * @param apply_shift   Whether the shift (odd-even) configuration should be used.
     * @return
     */
    template<typename VectorType>
    size_t optimizePartitions(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        TargetType target_type,
        size_t partition_height,
        size_t comparison_height,
        bool apply_shift
    )
    {
        using namespace shared;

        long partition_len = long(std::pow(2., partition_height - comparison_height));
        auto comparison_height_dims = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;

        std::pair<long, long> offset{ 0, 0 };
        std::pair<long, long> iteration_dims(comparison_height_dims);

        if (apply_shift) {  // Odd-even configuration - We basically shift the whole block back by the size of the partition
            offset.first -= partition_len;
            offset.second -= partition_len;
            iteration_dims.first += 2 * partition_len;
            iteration_dims.second += 2 * partition_len;
        }

        computeAggregates(quad_tree);
        return performPartitionExchanges(
            quad_tree,
            distance_function,
            target_type,
            partition_height,
            comparison_height,
            apply_shift,
            partition_len,
            offset,
            iteration_dims,
            comparison_height_dims
        );
    }
}

#endif //LDG_CORE_PARTITIONS_HPP
