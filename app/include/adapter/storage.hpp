#ifndef LDG_CORE_STORAGE_HPP
#define LDG_CORE_STORAGE_HPP

#include <string>
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/supertiles/util/place/data.hpp"
#include "data_layout.hpp"

namespace adapter
{
    /**
     * Save an assignment to a .raw.bz2 file.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param filename
     */
    template<typename VectorType>
    void saveAndCompressAssignment(shared::QuadAssignmentTree<VectorType> &quad_tree, std::string const filename)
    {
        std::vector<size_t> assignment(quad_tree.getAssignment());

//        helper::bzip_compress(qtLeafAssignment, po.outDir + "/qtLeafAssignment.raw.bz2");
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
        V2<size_t> grid_dim{ num_rows, num_cols };
        size_t required_assignment_capacity = shared::determineRequiredArrayCapacity(num_rows, num_cols);
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
