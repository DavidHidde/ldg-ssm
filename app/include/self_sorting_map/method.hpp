#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <algorithm>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/tree_functions.hpp"

namespace ssm
{
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
        std::vector<shared::TreeWalker<DataType>> &current_nodes,
        shared::QuadAssignmentTree<DataType> &quad_tree,
        size_t target_num_rows,
        size_t target_num_cols,
        size_t target_height
    )
    {
        using namespace shared;
        std::vector<TreeWalker<DataType>> next_nodes(
            target_num_cols * target_num_rows, TreeWalker<DataType>{
                CellPosition{ 0, 0 },
                1,
                1,
                quad_tree
            }
        ); // Temporarily assign a basically empty tree walker.
        for (shared::TreeWalker<DataType> node: current_nodes) {
            std::array<int, 4> child_indices = node.getChildrenIndices();
            for (int child_idx: child_indices) {
                if (child_idx >= 0) {
                    next_nodes[child_idx] = TreeWalker<DataType>{
                        CellPosition{ target_height, size_t(child_idx) },
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
        std::function<float(std::shared_ptr<DataType>, std::shared_ptr<DataType>)> distance_function,
        size_t max_height
    )
    {
        using namespace shared;
        // Load all the nodes and parents beforehand
        size_t num_nodes = nodes.size();
        size_t height_offset = quad_tree.getBounds(nodes[0]).first.first;
        auto &assignment = *(quad_tree.getAssignment());

        std::vector<size_t> node_assignments(num_nodes);                                // Assigned indices per node
        std::vector<std::shared_ptr<DataType>> node_data(num_nodes);                    // Actual data per node
        std::vector<std::shared_ptr<DataType>> parent_data(num_nodes * max_height);     // Data of the parents per node

        for (size_t idx = 0; idx < num_nodes; ++idx) {
            node_data[idx] = quad_tree.getValue(nodes[idx]);
            node_assignments[idx] = assignment[nodes[idx].index + height_offset];
            TreeWalker<DataType> walker{ nodes[idx], quad_tree };
            walker.moveUp();
            for (size_t parent_idx = 0; parent_idx < max_height - 1 && walker.moveUp(); ++parent_idx) {
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
                for (size_t parent_idx = 0; parent_idx < max_height - 1; ++parent_idx) {
                    distance += distance_function(
                        node_data[permutation[idx]],
                        parent_data[rowMajorIndex(parent_idx, idx, num_nodes)]
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
                assignment[nodes[idx].index + height_offset] = node_assignments[best_permutation[idx]];
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
        std::function<float(std::shared_ptr<DataType>, std::shared_ptr<DataType>)> distance_function,
        size_t max_height
    )
    {
        using namespace shared;
        size_t num_swaps = 0;
        std::vector<CellPosition> nodes(leaf_iterators.size());
        do {
            nodes.clear();
            for (auto &iterator: leaf_iterators) {
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

    /**
     * The self-sorting map as detailed in https://doi.org/10.1109/TMM.2014.2306183,
     * specifically for a QuadAssignmentTree given a distance function.
     *
     * TODO: Fix non-square grid looping
     *
     * @tparam DataType
     * @param quad_tree
     * @param distance_function
     */
    template<typename DataType>
    void sort(
        shared::QuadAssignmentTree<DataType> &quad_tree,
        std::function<float(std::shared_ptr<DataType>, std::shared_ptr<DataType>)> distance_function,
        const size_t max_iterations
    )
    {
        using namespace shared;
        size_t height = quad_tree.getDepth() - 1;
        size_t num_rows = 1;
        size_t num_cols = 1;
        float factor = std::pow(2., height);
        std::vector<TreeWalker<DataType>> current_nodes{ TreeWalker<DataType>{ CellPosition{ height, 0 }, num_rows, num_cols, quad_tree }};

        // Split up until we have at least 4x4 blocks.
        while (height > 0 && (num_rows < 4 || num_cols < 4)) {
            --height;
            factor /= 2.;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree.getNumCols(), factor);

            current_nodes = descend<DataType>(current_nodes, quad_tree, num_rows, num_cols, height);
        }

        // Main loop - We use the height as an indicator of the partition size rather than calculating the partition size.
        size_t iterations = 0;
        size_t num_swaps;
        size_t shift = 0;   // 0 == even-odd, 1 == odd-even
        for (; height > 0; --height) {
            do {
                num_swaps = 0;
                computeAggregates(quad_tree);

                // Solve (inner) 4x4 blocks
                for (size_t y = shift; y < num_rows - shift; y += 2) {
                    for (size_t x = shift; x < num_cols - shift; x += 2) {
                        size_t idx = rowMajorIndex(y, x, num_cols);
                        bool x_on_right_edge = x == num_cols - 1;
                        bool y_on_right_edge = y == num_rows - 1;

                        std::vector<RowMajorIterator<DataType>> iterators{ current_nodes[idx].getLeaves() };
                        if (!x_on_right_edge)
                            iterators.push_back(current_nodes[idx + 1].getLeaves());
                        if (!y_on_right_edge)
                            iterators.push_back(current_nodes[idx + num_cols].getLeaves());
                        if (!x_on_right_edge && !y_on_right_edge)
                            iterators.push_back(current_nodes[idx + num_cols + 1].getLeaves());

                        num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height + 1 + shift);
                    }
                }

                // Solve for edges in case of odd-even pairings
                if (shift) {
                    // Top & bottom edges
                    for (size_t y = 0; y < num_rows; y += num_rows - 1) {
                        for (size_t x = 1; x < num_cols - 1; x += 2) {
                            size_t idx = rowMajorIndex(y, x, num_cols);
                            std::vector<RowMajorIterator<DataType>> iterators{
                                current_nodes[idx].getLeaves(),
                                current_nodes[idx + 1].getLeaves()
                            };
                            num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height + 1 + shift);
                        }
                    }
                    // Left & right edges
                    for (size_t x = 0; x < num_cols; x += num_cols - 1) {
                        for (size_t y = 1; y < num_rows - 1; y += 2) {
                            size_t idx = rowMajorIndex(y, x, num_cols);
                            std::vector<RowMajorIterator<DataType>> iterators{
                                current_nodes[idx].getLeaves(),
                                current_nodes[idx + num_cols].getLeaves()
                            };
                            num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height + 1 + shift);
                        }
                    }
                }

                shift = shift == 0 ? 1 : 0;
                std::cout << "height: " << height << ", iteration: " << iterations << ", num_swaps : " << num_swaps << '\n';
                ++iterations;
            } while (iterations < max_iterations && num_swaps > 0);

            // Descend to the next level
            factor /= 2;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree.getNumCols(), factor);
            current_nodes = descend<DataType>(current_nodes, quad_tree, num_rows, num_cols, height - 1);

            // Reset counts
            iterations = 0;
            shift = 0;
        }
    }
}

#endif //LDG_CORE_METHOD_HPP
