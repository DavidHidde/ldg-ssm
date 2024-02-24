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
                num_swaps += optimizePartitions(quad_tree, distance_function, TargetType::NEIGHBOURHOOD, height, 0, false);
                num_swaps += optimizePartitions(quad_tree, distance_function, TargetType::NEIGHBOURHOOD, height, 0, true);
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
