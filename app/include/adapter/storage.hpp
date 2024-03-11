#ifndef LDG_CORE_STORAGE_HPP
#define LDG_CORE_STORAGE_HPP

#include <string>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/supertiles/util/place/data.hpp"
#include "data_layout.hpp"

namespace adapter
{
    /**
     * Save an assignment to a .raw.bz2 file.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param file_name
     */
    template<typename VectorType>
    void saveAndCompressAssignment(ldg::QuadAssignmentTree<VectorType> &quad_tree, std::string const file_name)
    {
        size_t grid_side_len = std::pow(2, std::ceil(std::log2(std::max(quad_tree.getNumRows(), quad_tree.getNumCols()))));
        std::vector assignment(grid_side_len * grid_side_len, supertiles::place::voidTileIdx);
        copyFromRowMajorToHierarchy(quad_tree.getAssignment(), assignment, quad_tree.getNumRows(), quad_tree.getNumCols());

        // Add void tiles where the data refers to nullptrs
        for (uint32_t &value: assignment) {
            if (value != supertiles::place::voidTileIdx && quad_tree.getValue(ldg::CellPosition{ 0, value }) == nullptr) {
                value = supertiles::place::voidTileIdx;
            }
        }

        helper::bzip_compress(assignment, file_name + ".raw.bz2");
    }

    /**
     * Read an assignment from a .raw.bz2 file.
     *
     * @param filename
     * @param num_rows
     * @param num_cols
     * @param num_actual_elements
     * @return
     */
    std::vector<size_t> readCompressedAssignment(std::string const filename, size_t num_rows, size_t num_cols, size_t num_actual_elements)
    {
        V2 grid_dim{ num_rows, num_cols };
        size_t required_assignment_capacity = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
        std::vector<uint32_t> hierarchical_assignment = supertiles::place::read_qtLeafAssignment(filename, grid_dim);
        std::vector<size_t> row_major_assignment(required_assignment_capacity);
        copyFromHierarchyToRowMajor(hierarchical_assignment, row_major_assignment, num_rows, num_cols);

        // Replace all void tiles with the end of the data array (assumed to be nullptrs) and give aggregates an assignment.
        size_t next_void_tile_placement = num_actual_elements;
        size_t num_leafs = num_rows * num_cols;
        for (size_t idx = 0; idx < required_assignment_capacity; ++idx) {
            if (row_major_assignment[idx] == supertiles::place::voidTileIdx) {
                row_major_assignment[idx] = next_void_tile_placement++;
            }
            if (idx >= num_leafs) {
                row_major_assignment[idx] = idx;
            }
        }

        return row_major_assignment;
    }
}

#endif //LDG_CORE_STORAGE_HPP
