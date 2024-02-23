#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <algorithm>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"
#include "app/include/shared/tree_functions.hpp"
#include "app/include/shared/util/image.hpp"
#include "targets.hpp"
#include "partitions.hpp"

namespace ssm
{
    /**
     * Descend the tree to the next level by creating new walkers for all children of the current walkers.
     *
     * @tparam VectorType
     * @param current_nodes
     * @param quad_tree
     * @param target_num_rows
     * @param target_num_cols
     * @param target_height
     * @return
     */
    template<typename VectorType>
    std::vector<shared::TreeWalker<VectorType>> descend(
        std::vector<shared::TreeWalker<VectorType>> &current_nodes,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t target_num_rows,
        size_t target_num_cols,
        size_t target_height
    )
    {
        using namespace shared;
        std::vector<TreeWalker<VectorType>> next_nodes(
            target_num_cols * target_num_rows,
            TreeWalker<VectorType>{
                CellPosition{ 0, 0 },
                1,
                1,
                quad_tree
            }
        ); // Temporarily assign a basically empty tree walker.
        for (shared::TreeWalker<VectorType> node: current_nodes) {
            std::array<int, 4> child_indices = node.getChildrenIndices();
            for (int child_idx: child_indices) {
                if (child_idx >= 0) {
                    next_nodes[child_idx] = TreeWalker<VectorType>{
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
     * Perform swaps between partitions by iterating over the leafs. Remove a partition once it is empty.
     *
     * @tparam VectorType
     * @param leaf_iterators
     * @param quad_tree
     * @param distance_function
     * @param min_height
     * @param max_height
     * @return
     */
    template<typename VectorType>
    size_t performSwapsWithPartitions(
        std::vector<shared::RowMajorIterator<VectorType>> &leaf_iterators,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        size_t current_height,
        bool is_shift
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
            if (!nodes.empty()) {
                auto target_data = getTargets(TargetType::HIERARCHY, nodes, quad_tree, current_height, is_shift);
                num_swaps += findAndSwapBestPermutation(nodes, quad_tree, distance_function, target_data.first, target_data.second);
            }
        } while (!nodes.empty());

        return num_swaps;
    }

    /**
     * Partition into even-odd or odd-even pairings and start the swaps.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param current_nodes
     * @param distance_function
     * @param num_rows
     * @param num_cols
     * @param height
     * @param shift
     * @return
     */
    template<typename VectorType>
    size_t findAndSwapPartitions(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::vector<shared::TreeWalker<VectorType>> &current_nodes,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        size_t num_rows,
        size_t num_cols,
        size_t height,
        size_t shift
    )
    {
        using namespace shared;
        size_t num_swaps = 0;
        computeAggregates(quad_tree);

        // Solve (inner) 4x4 blocks
        for (size_t y = shift; y < num_rows - shift; y += 2) {
            for (size_t x = shift; x < num_cols - shift; x += 2) {
                size_t idx = rowMajorIndex(y, x, num_cols);
                bool x_on_right_edge = x == num_cols - 1;
                bool y_on_right_edge = y == num_rows - 1;

                std::vector<RowMajorIterator<VectorType>> iterators{ current_nodes[idx].getLeaves() };
                if (!x_on_right_edge)
                    iterators.push_back(current_nodes[idx + 1].getLeaves());
                if (!y_on_right_edge)
                    iterators.push_back(current_nodes[idx + num_cols].getLeaves());
                if (!x_on_right_edge && !y_on_right_edge)
                    iterators.push_back(current_nodes[idx + num_cols + 1].getLeaves());

                num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height, shift != 0);
            }
        }

        // Solve for edges in case of odd-even pairings
        if (shift) {
            // Top & bottom edges
            for (size_t y = 0; y < num_rows; y += num_rows - 1) {
                for (size_t x = 1; x < num_cols - 1; x += 2) {
                    size_t idx = rowMajorIndex(y, x, num_cols);
                    std::vector<RowMajorIterator<VectorType>> iterators{
                        current_nodes[idx].getLeaves(),
                        current_nodes[idx + 1].getLeaves()
                    };
                    num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height, true);
                }
            }
            // Left & right edges
            for (size_t x = 0; x < num_cols; x += num_cols - 1) {
                for (size_t y = 1; y < num_rows - 1; y += 2) {
                    size_t idx = rowMajorIndex(y, x, num_cols);
                    std::vector<RowMajorIterator<VectorType>> iterators{
                        current_nodes[idx].getLeaves(),
                        current_nodes[idx + num_cols].getLeaves()
                    };
                    num_swaps += performSwapsWithPartitions(iterators, quad_tree, distance_function, height, true);
                }
            }
        }

        return num_swaps;
    }

    /**
     * The self-sorting map as detailed in https://doi.org/10.1109/TMM.2014.2306183,
     * specifically for a QuadAssignmentTree given a distance function.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     */
    template<typename VectorType>
    void sort(
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        const size_t max_iterations
    )
    {
        using namespace shared;
        size_t height = quad_tree.getDepth() - 1;
        size_t num_rows = 1;
        size_t num_cols = 1;
        double factor = std::pow(2., height);
        std::vector<TreeWalker<VectorType>> current_nodes{
            TreeWalker<VectorType>{ CellPosition{ height, 0 }, num_rows, num_cols, quad_tree }};

        // Split up until we have at least 4x4 blocks.
        while (height > 0 && (num_rows < 4 || num_cols < 4)) {
            --height;
            factor /= 2.;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree.getNumCols(), factor);

            current_nodes = descend<VectorType>(current_nodes, quad_tree, num_rows, num_cols, height);
        }

        // Main loop - We use the height as an indicator of the partition size rather than calculating the partition size.
        size_t iterations = 0;
        size_t num_swaps;
        for (; height > 0; --height) {
            do {
                num_swaps = 0;
                num_swaps += optimizePartitions(quad_tree, distance_function, TargetType::HIERARCHY, height, 0, false);
                num_swaps += optimizePartitions(quad_tree, distance_function, TargetType::HIERARCHY, height, 0, true);
                ++iterations;
            } while (iterations < max_iterations && num_swaps > 0);
            saveQuadTreeImages(quad_tree, "ssm-size(" + std::to_string(size_t(std::pow(2., height))) + ")");
            std::cout << "Finished height " << height << " in " << iterations << " iterations \n";

            // Descend to the next level
            factor /= 2;
            num_rows = ceilDivideByFactor(quad_tree.getNumRows(), factor);
            num_cols = ceilDivideByFactor(quad_tree.getNumCols(), factor);
            current_nodes = descend<VectorType>(current_nodes, quad_tree, num_rows, num_cols, height - 1);

            // Reset counts
            iterations = 0;
        }

        if (num_swaps > 0)
            computeAggregates(quad_tree);   // Fix aggregates in the end if we did perform swaps in the end.
    }
}

#endif //LDG_CORE_METHOD_HPP
