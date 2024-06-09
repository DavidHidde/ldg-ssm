#ifndef LDG_CORE_PARTITIONS_HPP
#define LDG_CORE_PARTITIONS_HPP

#include <functional>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/exchanges.hpp"

namespace ssm
{
    /**
     * Gather [a, b] nodes starting from a top-left (x, y) coordinate and perform exchanges between them.
     *
     * @tparam VectorType
     * @param x
     * @param y
     * @param stride
     * @param height
     * @param elems_per_dim [nodes in y dimension, nodes in x dimension]
     * @param comparison_height_dims
     * @param node_buffer
     * @param quad_tree
     * @param distance_function
     * @param target_map
     * @return
     */
    template<typename VectorType>
    size_t pairNodesAndPerformExchanges(
        const long x,
        const long y,
        const long stride,
        const size_t height,
        std::pair<size_t, size_t> &elems_per_dim,
        std::pair<size_t, size_t> &comparison_height_dims,
        std::vector<ldg::CellPosition> &node_buffer,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map
    ) {
        auto [num_y_elems, num_x_elems] = elems_per_dim;
        auto [num_rows, num_cols] = comparison_height_dims;
        node_buffer.clear();

        long node_x;
        long node_y = y;
        for (long y_idx = 0; y_idx < num_y_elems && node_y < static_cast<long>(num_rows); ++y_idx, node_y += stride) {
            node_x = x;
            for (long x_idx = 0; x_idx < num_x_elems && node_x < static_cast<long>(num_cols); ++x_idx, node_x += stride) {
                // Check if the indices are exist and add the node if they do. The upper ranges for x and y are guarded by the for-loop.
                if (node_x >= 0 && node_y >= 0) {
                    node_buffer.push_back(ldg::CellPosition{ height, ldg::rowMajorIndex(node_y, node_x, num_cols) });
                }
            }
        }

        return node_buffer.size() > 1 ? findAndSwapBestPermutation(node_buffer, quad_tree, distance_function, target_map) : 0;
    }

    /**
     * Perform the exchanges of the self-sorting map. This functions handles pairing up the right data and then getting it compared.
     * This functions goes over the data without the use of the fancy iterators to allow easy element-wise comparisons for better
     * parallelization. Additionally, this iterates in a row-major fashion which results in better data reading.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param target_map
     * @param comparison_height
     * @param partition_len Length of the current partition.
     * @param offset    Offset [rows, columns] for the calculated indices. This is used to project back to actual array indices
     * @param iteration_dims    The dimensions to be iterated over.
     * @param comparison_height_dims The dimensions at the height of comparison.
     * @param elements_per_dim Number of elements that should be paired per dimension, defined in [elements per y, elements per x]
     * @return
     */
    template<typename VectorType>
    size_t performPartitionExchanges(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        const size_t comparison_height,
        const long partition_len,
        std::pair<long, long> &offset,
        std::pair<long, long> &iteration_dims,
        std::pair<size_t, size_t> &comparison_height_dims,
        std::pair<size_t, size_t> &elements_per_dim
    )
    {
        using namespace ldg;

        auto [iteration_num_rows, iteration_num_cols] = iteration_dims;
        auto [elements_per_y, elements_per_x] = elements_per_dim;
        auto [offset_y, offset_x] = offset;

        long projected_num_rows = iteration_num_rows / elements_per_y + (iteration_num_rows % (elements_per_y * partition_len)) % partition_len;
        long projected_num_cols = iteration_num_cols / elements_per_x + (iteration_num_cols % (elements_per_x * partition_len)) % partition_len;
        long num_elems = projected_num_rows * projected_num_cols;

        std::vector<CellPosition> nodes;
        nodes.reserve(elements_per_x * elements_per_y);
        size_t num_exchanges = 0;

#pragma omp parallel for private(nodes) reduction(+:num_exchanges) schedule(static)
        for (long idx = 0; idx < num_elems; ++idx) {
            long projected_x = idx % projected_num_cols;
            long projected_y = idx / projected_num_cols;

            long partition_x = projected_x / partition_len;
            long partition_y = projected_y / partition_len;
            long within_partition_x = projected_x % partition_len;
            long within_partition_y = projected_y % partition_len;

            long base_x = offset_x + within_partition_x + partition_x * partition_len * elements_per_x;
            long base_y = offset_y + within_partition_y + partition_y * partition_len * elements_per_y;

            num_exchanges += pairNodesAndPerformExchanges(
                base_x,
                base_y,
                partition_len,
                comparison_height,
                elements_per_dim,
                comparison_height_dims,
                nodes,
                quad_tree,
                distance_function,
                target_map
            );
        }

        return num_exchanges;
    }

    /**
     * Start partition optimization by performing exchanges. This function in particular applies the even-odd or odd-even swapping.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param target_types   Types of targets for the target comparisons
     * @param partition_height  The height of the partitions being compared.
     * @param comparison_height The height at which the elements should be compared.
     * @param elements_per_dim Number of elements that should be paired per dimension, defined in [elements per y, elements per x]
     * @param apply_shift   Whether the shift (odd-even) configuration should be used.
     * @return
     */
    template<typename VectorType>
    size_t optimizePartitions(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::vector<TargetType> const &target_types,
        size_t partition_height,
        size_t comparison_height,
        std::pair<size_t, size_t> &elements_per_dim,
        bool apply_shift
    )
    {
        using namespace ldg;

        long partition_len = long(std::pow(2., partition_height - comparison_height));
        auto comparison_height_dims = quad_tree.getBounds(comparison_height).second;

        std::pair<long, long> offset{ 0, 0 };
        std::pair<long, long> iteration_dims(comparison_height_dims);

        if (apply_shift) {  // Odd-even configuration - We basically shift the whole block back by the size of the partition
            auto [elements_per_y, elements_per_x] = elements_per_dim;
            offset.first -= (elements_per_y - 1) * partition_len;
            offset.second -= (elements_per_x - 1) * partition_len;
            iteration_dims.first += (elements_per_y - 1) * 2 * partition_len;
            iteration_dims.second += (elements_per_x - 1) * 2 * partition_len;
        }

        computeAggregates(quad_tree);
        auto target_map = getTargetMap(target_types, quad_tree, partition_height, comparison_height, apply_shift);
        return performPartitionExchanges(
            quad_tree,
            distance_function,
            target_map,
            comparison_height,
            partition_len,
            offset,
            iteration_dims,
            comparison_height_dims,
            elements_per_dim
        );
    }
}

#endif //LDG_CORE_PARTITIONS_HPP
