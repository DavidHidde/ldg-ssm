#include "app/include/self_sorting_map/method.hpp"

/**
 * Descend the tree to the next level by creating new walkers for all children of the current walkers.
 *
 * @tparam DataType
 * @param current_nodes
 * @param quad_tree
 * @param target_num_rows
 * @param target_num_cols
 * @param target_height
 * @return
 */
template<typename DataType>
std::vector<shared::TreeWalker<DataType>> descend(
    std::vector<shared::TreeWalker<DataType>> const &current_nodes,
    shared::QuadAssignmentTree<DataType> const &quad_tree,
    const size_t target_num_rows,
    const size_t target_num_cols,
    const size_t target_height
)
{
    using namespace shared;
    std::vector<TreeWalker<DataType>> next_nodes(target_num_cols * target_num_rows);
    for (size_t idx = 0; idx < current_nodes.size(); ++idx) {
        auto child_indices = current_nodes[idx].getChildrenIndices();
        for (size_t child_idx = 0; child_idx < child_indices.size(); ++child_idx) {
            int actual_index = child_indices[child_idx];
            if (actual_index >= 0) {
                next_nodes[actual_index] = TreeWalker<DataType>{
                    CellPosition{ target_height, size_t(actual_index) },
                    target_num_rows,
                    target_num_cols,
                    quad_tree
                };
            }
        }
    }
    return next_nodes;
}

/**
 * Compare nodes and find the permutation which minimizes the distance to all parents.
 * Afterwards, swap all items into this permutation.
 *
 * @tparam DataType
 * @param nodes
 * @param quad_tree
 * @param distance_function
 * @param max_height The maximum parent height to compare to.
 * @return The number of swaps performed.
 */
template<typename DataType>
size_t findAndSwapBestPermutation(
    std::vector<shared::CellPosition> nodes,
    shared::QuadAssignmentTree<DataType> &quad_tree,
    std::function<float(DataType, DataType)> distance_function,
    size_t max_height
)
{
    using namespace shared;
    // Load all the nodes and parents beforehand
    size_t num_nodes = nodes.size();
    auto assignment = *(quad_tree.getAssignment());
    std::vector<DataType &> node_data(num_nodes);
    std::vector<size_t> node_assignments(num_nodes);
    std::vector<DataType &> parent_data{ num_nodes * max_height };
    for (size_t idx = 0; idx < num_nodes; ++idx) {
        node_data[idx] = quad_tree.getValue(nodes[idx]);
        node_assignments[idx] = assignment[nodes[idx].index];
        TreeWalker<DataType> walker{ nodes[idx], quad_tree };
        for (size_t parent_idx = 0; parent_idx < max_height && walker.moveUp(); ++parent_idx) {
            parent_data[rowMajorIndex(parent_idx, idx, num_nodes)] = walker.getNodeValue();
        }
    }

    // Find best permutation
    float minimum_distance = -1.; // We assume the distance to always be positive, so this flags this to uninitialized.
    std::vector<size_t> best_permutation(num_nodes);
    for (size_t idx = 0; idx < num_nodes; ++idx)
        best_permutation[idx] = idx;
    std::vector<size_t> permutation(best_permutation);

    do {
        float distance = 0.;
        for (size_t idx = 0; idx < num_nodes; ++idx) {
            for (size_t parent_idx = 0; parent_idx < max_height; ++parent_idx) {
                distance += distance_function(
                    node_data[permutation[idx]],
                    parent_data[rowMajorIndex(parent_idx, node_data[permutation[idx]], 4)]
                );
            }

            // Better permutation, save it
            if (minimum_distance == -1 || minimum_distance > distance) {
                minimum_distance = distance;
                best_permutation = permutation;
            }
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));

    // Perform the actual swapping where needed
    size_t swap_count = 0;
    for (size_t idx = 0; idx < num_nodes; ++idx) {
        if (best_permutation[idx] != idx) {
            assignment[nodes[idx].index] = node_assignments[best_permutation[idx]];
            ++swap_count;
        }
    }
    return swap_count;
}

/**
 * Perform swaps between partitions by iterating over the leafs. Remove a partition once it is empty.
 *
 * @tparam DataType
 * @param leaf_iterators
 * @param quad_tree
 * @param distance_function
 * @return
 */
template<typename DataType>
size_t performSwapsWithPartitions(
    std::vector<shared::RowMajorIterator<DataType>> &leaf_iterators,
    shared::QuadAssignmentTree<DataType> &quad_tree,
    std::function<float(DataType, DataType)> distance_function,
    size_t max_height
)
{
    using namespace shared;
    size_t num_swaps = 0;
    std::vector<bool> iterators_finished(leaf_iterators.size());
    std::vector<CellPosition> nodes(leaf_iterators.size());
    do {
        nodes.clear();
        for (auto iterator: leaf_iterators) {
            // Ignore finished partitions
            if (iterator != iterator.end()) {
                nodes.push_back(iterator.getPosition());
                ++iterator;
            }
        }
        if (!nodes.empty())
            num_swaps += findAndSwapBestPermutation(nodes, quad_tree, distance_function, max_height);
    } while (!nodes.empty());

    return num_swaps;
}

namespace ssm
{
    /**
     * The self-sorting map as detailed in https://doi.org/10.1109/TMM.2014.2306183,
     * specifically for a QuadAssignmentTree given a distance function.
     *
     * @tparam DataType
     * @param quad_tree
     * @param distance_function
     */
    template<typename DataType>
    void sort(
        shared::QuadAssignmentTree<DataType> &quad_tree,
        std::function<float(DataType, DataType)> distance_function,
        const size_t max_iterations
    )
    {
        using namespace shared;
        std::vector<TreeWalker<DataType>> current_nodes{ TreeWalker<DataType>{ CellPosition{ 0, 0 }, quad_tree }};
        size_t height = quad_tree.getDepth() - 1;
        size_t num_rows = current_nodes[0].getNumRows();
        size_t num_cols = current_nodes[0].getNumCols();
        float factor = std::pow(2, height);

        // Split up until we have at least 4x4 blocks.
        while (height > 0 && (num_rows < 4 || num_cols < 4)) {
            --height;
            factor /= 2;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree->getNumCols(), factor);
            current_nodes = descend<DataType>(current_nodes, quad_tree, num_rows, num_cols, height);
        }

        // Main loop - We use the height as an indicator of the partition size rather than calculating the partition size.
        size_t iterations = 0;
        size_t num_swaps;
        size_t expected_remainder = 0;
        for (; height > 0; --height) {
            do {
                num_swaps = 0;
                quad_tree.computeAggregates();

                // Determine partitions and swap
                for (size_t idx = 0; idx < current_nodes.size(); ++idx) {
                    size_t x = idx % num_cols;
                    size_t y = idx / num_cols;
                    bool x_on_edge = x == num_cols - 1;
                    bool y_on_edge = y == num_rows - 1;
                    bool x_must_pair = x % 2 == expected_remainder;
                    bool y_must_pair = y % 2 == expected_remainder;

                    if ((x_must_pair && !x_on_edge) || (y_must_pair && !y_on_edge)) {
                        std::vector<RowMajorIterator<DataType>> iterators{ current_nodes[idx].getLeaves() };
                        if (x_must_pair && !x_on_edge)
                            iterators.push_back(current_nodes[idx + 1].getLeaves());
                        if (y_must_pair && !y_on_edge)
                            iterators.push_back(current_nodes[idx + num_cols].getLeaves());
                        if (x_must_pair && !x_on_edge && y_must_pair && !y_on_edge)
                            iterators.push_back(current_nodes[idx + num_cols + 1].getLeaves());
                        num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height + 1 + expected_remainder);
                    }
                }

                expected_remainder = expected_remainder == 0 ? 1 : 0;
                ++iterations;
            } while (iterations < max_iterations && num_swaps > 0);

            // Descend to the next level
            factor /= 2;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree->getNumCols(), factor);
            current_nodes = descend<DataType>(current_nodes, quad_tree, num_rows, num_cols, height - 1);

            // Reset counts
            iterations = 0;
            expected_remainder = 0;
        }
    }
}