#ifndef LDG_CORE_CELL_NEIGHBOURHOOD_TARGET_HPP
#define LDG_CORE_CELL_NEIGHBOURHOOD_TARGET_HPP

#include <cmath>
#include "app/include/ldg/model/cell_position.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"

namespace ssm
{
    size_t CELL_NUM_BLOCKS_PER_DIMENSION = 9;

    /**
     * Load the neighbourhood targets into a target array.
     * The cell neighbourhood target basically applies a convolutional filter over the grid at comparison_height to aggregate neighbours..
     *
     * @tparam VectorType
     * @param target_map
     * @param quad_tree
     * @param partition_height
     * @param comparison_height
     * @param is_shift
     */
    template<typename VectorType>
    void loadCellNeighbourhoodTargets(
        std::vector<std::vector<std::shared_ptr<VectorType>>> &target_map,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        const size_t partition_height,
        const size_t comparison_height,
        bool is_shift
    )
    {
        using namespace ldg;
        auto comparison_height_dims = quad_tree.getBounds(CellPosition{ comparison_height, 0 }).second;
        size_t num_elems = comparison_height_dims.first * comparison_height_dims.second;
        int cells_before = (CELL_NUM_BLOCKS_PER_DIMENSION - 1) / 2;
        int cells_after = cells_before + (CELL_NUM_BLOCKS_PER_DIMENSION - 1) % 2;
        std::vector<std::shared_ptr<VectorType>> values;
        values.reserve(CELL_NUM_BLOCKS_PER_DIMENSION * CELL_NUM_BLOCKS_PER_DIMENSION);

#pragma omp parallel for private(values) schedule(static)
        for (size_t idx = 0; idx < num_elems; ++idx) {
            int cell_x = idx % comparison_height_dims.second;
            int cell_y = idx / comparison_height_dims.second;

            size_t min_y = std::max(cell_y - cells_before, 0);
            size_t max_y = std::min(size_t(cell_y + cells_after), comparison_height_dims.first);
            size_t min_x = std::max(cell_x - cells_before, 0);
            size_t max_x = std::min(size_t(cell_x + cells_after), comparison_height_dims.second);

            for (size_t y = min_y; y < max_y; ++y) {
                for (size_t x = min_x; x < max_x; ++x) {
                    values.push_back(
                        quad_tree.getValue(
                            CellPosition{ partition_height, rowMajorIndex(y, x, comparison_height_dims.second) }
                        ));
                }
            }
            target_map[idx].push_back(std::make_shared<VectorType>(aggregate(values, quad_tree.getDataElementLen())));
            values.clear();
        }
    }
}

#endif //LDG_CORE_CELL_NEIGHBOURHOOD_TARGET_HPP
