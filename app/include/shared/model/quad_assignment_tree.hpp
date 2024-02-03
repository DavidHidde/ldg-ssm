#ifndef IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
#define IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP

#include <vector>
#include <cstddef>
#include <memory>
#include <cmath>

namespace shared
{
    /**
     * Enum of all quadrants of a square.
     */
    enum Quadrant
    {
        NORTH_WEST,
        NORTH_EAST,
        SOUTH_WEST,
        SOUTH_EAST
    };

    /**
     * Simple POD to indicate the position of a cell in the quad tree.
     */
    struct CellPosition {
        size_t height;
        size_t index;
    };

    /**
     * Quad tree of the data. Uses a flat row-major data array for all heights of the tree, which should already exist.
     * Uses an assignment array to determine the grid assignment.
     * TODO: support non-square grids.
     * @tparam DataType The data type of the grid.
     */
    template<typename DataType>
    class QuadAssignmentTree
    {
        size_t num_rows;
        size_t num_cols;
        size_t depth;

        std::shared_ptr<std::vector<DataType>> data;
        std::shared_ptr<std::vector<size_t>> assignment;

    public:
        QuadAssignmentTree(const std::shared_ptr<std::vector<DataType>> &data, size_t num_rows, size_t num_cols, size_t depth);

        size_t &getDepth();

        size_t &getNumRows();

        size_t &getNumCols();

        std::shared_ptr<std::vector<size_t>> getAssignment();

        std::shared_ptr<std::vector<DataType>> getData();

        DataType &getValue(CellPosition &position);

        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> getBounds(CellPosition &position);

        void computeAggregates();
    };
}

#endif //IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
