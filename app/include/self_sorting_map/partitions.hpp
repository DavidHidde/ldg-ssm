#ifndef LDG_CORE_PARTITIONS_HPP
#define LDG_CORE_PARTITIONS_HPP

#include <functional>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/self_sorting_map/exchanges.hpp"

namespace ssm
{
    /**
     * Generate the pairings of each cell in a partition to each other cell.
     * If in SSM mode, then the cell of each partition should correspond to the same cell in another partition.
     * If not, randomize the pairings to avoid local minima through bad pairings.
     *
     * @param num_elements
     * @param randomize
     * @return
     */
    std::array<std::vector<long>, 4> generateCellPairings(size_t num_elements, bool randomize)
    {
        // Create maps per element
        std::vector<long> map(num_elements);
        std::iota(map.begin(), map.end(), 0);

        // Copy simple ranges into array
        std::array<std::vector<long>, 4> pair_array{
            map,
            map,
            map,
            map
        };

        // Shuffle all elements if not in SSM mode
        if (randomize) {
            for (auto &indices : pair_array) {
                std::shuffle(indices.begin(), indices.end(), program::RANDOMIZER);
            }
        }

        return pair_array;
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
     * @param cell_pairings_array
     * @param partition_len Length of the current partition.
     * @param offset    Offset [rows, columns] for the calculated indices. This is used to project back to actual array indices
     * @param iteration_dims    The dimensions to be iterated over.
     * @return
     */
    template<typename VectorType>
    size_t performPartitionExchanges(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        std::array<std::vector<long>, 4> &cell_pairings_array,
        const long partition_len,
        std::pair<long, long> &offset,
        std::pair<long, long> &iteration_dims
    ) {
        using namespace ldg;

        auto [iteration_num_rows, iteration_num_cols] = iteration_dims;
        auto [comparison_num_rows, comparison_num_cols] = quad_tree.getBounds(0).second;
        auto [offset_y, offset_x] = offset;

        long projected_num_rows = iteration_num_rows / 2 + (iteration_num_rows % (2 * partition_len)) % partition_len;
        long projected_num_cols = iteration_num_cols / 2 + (iteration_num_cols % (2 * partition_len)) % partition_len;
        long num_elems = projected_num_rows * projected_num_cols;

        std::vector<CellPosition> nodes;
        nodes.reserve(4);
        size_t num_exchanges = 0;

#pragma omp parallel for private(nodes) reduction(+:num_exchanges) schedule(static)
        for (long idx = 0; idx < num_elems; ++idx) {
            long projected_x = idx % projected_num_cols;
            long projected_y = idx / projected_num_cols;

            long partition_x = projected_x / partition_len;
            long partition_y = projected_y / partition_len;

            long within_partition_x = projected_x % partition_len;
            long within_partition_y = projected_y % partition_len;
            long within_partition_index = ldg::rowMajorIndex(within_partition_y, within_partition_x, partition_len);

            long base_x = offset_x + partition_x * partition_len * 2;
            long base_y = offset_y + partition_y * partition_len * 2;

            // Pair nodes and perform exchanges
            nodes.clear();
            long count = 0;   // Use a count to adjust for selecting the neighbouring partitions
            for (auto &cell_pairings : cell_pairings_array) {
                long pair_index = cell_pairings[within_partition_index];
                long pair_x = base_x + pair_index % partition_len + (count % 2) * partition_len;
                long pair_y = base_y + pair_index / partition_len + (count / 2) * partition_len;

                // Check if this node is within range
                if (pair_x >= 0 && pair_x < comparison_num_cols && pair_y >= 0 && pair_y < comparison_num_rows) {
                    nodes.push_back(ldg::CellPosition{ 0, ldg::rowMajorIndex(pair_y, pair_x, comparison_num_cols) });
                }
                ++count;
            }
            num_exchanges += nodes.size() > 1 ? findAndSwapBestPermutation(nodes, quad_tree, distance_function, target_map) : 0;
        }

        return num_exchanges;
    }

    /**
     * Start partition optimization by performing exchanges. This function in particular applies the even-odd or odd-even swapping.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param partition_height  The height of the partitions being compared.
     * @param ssm_mode
     * @param apply_shift   Whether the shift (odd-even) configuration should be used.
     * @return
     */
    template<typename VectorType>
    size_t optimizePartitions(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        const size_t partition_height,
        const bool ssm_mode,
        const bool apply_shift
    ) {
        using namespace ldg;

        long partition_len = long(std::pow(2., partition_height));
        std::pair<long, long> offset{ 0, 0 };
        std::pair<long, long> iteration_dims(quad_tree.getBounds(0).second);

        if (apply_shift) {  // Odd-even configuration - We basically shift the whole block back by the size of the partition
            offset.first -= partition_len;
            offset.second -= partition_len;
            iteration_dims.first += 2 * partition_len;
            iteration_dims.second += 2 * partition_len;
        }

        computeParents(quad_tree, distance_function);
        auto target_map = getTargetMap(
            ssm_mode ? TargetType::PARTITION_NEIGHBOURHOOD : TargetType::HIGHEST_PARENT_HIERARCHY,
            quad_tree,
            distance_function,
            partition_height,
            apply_shift
        );
        auto cell_pairing_array= generateCellPairings(partition_len * partition_len, !ssm_mode);

        return performPartitionExchanges(
            quad_tree,
            distance_function,
            target_map,
            cell_pairing_array,
            partition_len,
            offset,
            iteration_dims
        );
    }
}

#endif //LDG_CORE_PARTITIONS_HPP
