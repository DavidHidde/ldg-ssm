#ifndef LDG_CORE_4_CONNECTED_TARGET_HPP
#define LDG_CORE_4_CONNECTED_TARGET_HPP

#include <functional>
#include <vector>
#include <memory>
#include "app/include/shared/model/quad_assignment_tree.hpp"

namespace ssm
{
    /**
     * Load the 4 connected variant of a target.
     * This aggregates the 4-connected neighbourhood of a node together with the node itself using a target function.
     *
     * @tparam VectorType
     * @tparam FunctionType
     * @param target_function
     * @param target_map
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     */
    template<typename VectorType, typename FunctionType>
    void load4ConnectedTarget(
        FunctionType target_function,
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        shared::QuadAssignmentTree<VectorType> &quad_tree,
        size_t partition_height,
        size_t comparison_height,
        bool is_shift
    )
    {
        using namespace shared;
        std::vector<std::vector<std::shared_ptr<VectorType>>> temporary_targets(target_map.size());
        target_function(temporary_targets, quad_tree, partition_height, comparison_height, is_shift);

        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(4);

        auto [num_rows, num_cols] = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        size_t num_elems = num_rows * num_cols;
#pragma omp parallel for private(values)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            size_t x = idx % num_cols;
            size_t y = idx / num_cols;

            if (x > 0)
                values.push_back(temporary_targets[rowMajorIndex(y, x - 1, num_cols)][0]);
            if (x < num_cols - 1)
                values.push_back(temporary_targets[rowMajorIndex(y, x + 1, num_cols)][0]);
            if (y > 0)
                values.push_back(temporary_targets[rowMajorIndex(y - 1, x, num_cols)][0]);
            if (y < num_rows - 1)
                values.push_back(temporary_targets[rowMajorIndex(y + 1, x, num_cols)][0]);

            // Aggregate neighbours, get target for current idx and aggregate into final target.
            target_map[rowMajorIndex(y, x, num_cols)].push_back(std::make_shared<VectorType>(aggregate(values, quad_tree.getDataElementLen())));
            target_map[rowMajorIndex(y, x, num_cols)].push_back(temporary_targets[rowMajorIndex(y, x, num_cols)][0]);
            values.clear();
        }
    }
}

#endif //LDG_CORE_4_CONNECTED_TARGET_HPP
