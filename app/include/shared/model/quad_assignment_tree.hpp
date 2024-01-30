#ifndef IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
#define IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP

#include <vector>
#include <cstddef>
#include <memory>

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
        SOUTH_EAST,
        NONE
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
        /**
         * Simple iterator for iterating over row-major arrays.
         */
        class RowMajorIterator
        {
            size_t grid_num_cols;
            size_t projected_num_rows;
            size_t projected_num_cols;
            size_t offset;
            std::shared_ptr<std::vector<DataType>> data;
            std::shared_ptr<std::vector<size_t>> assignment;

            size_t index;

            size_t currentIndex();

        public:
            RowMajorIterator begin();
            RowMajorIterator end();

            DataType & getValue();

            bool operator==(RowMajorIterator const &rhs);
            RowMajorIterator &operator++();
        };

        size_t num_rows;
        size_t num_cols;
        size_t depth;

        std::shared_ptr <std::vector<DataType>> data;
        std::shared_ptr <std::vector<size_t>> assignment;

    public:
        QuadAssignmentTree(std::shared_ptr<std::vector<DataType>> data);

        size_t getDepth();
        size_t getNumRows();
        size_t getNumCols();
        std::vector<size_t> *getAssignment();


    };
}

#endif //IMPROVED_LDG_QUAD_ASSIGNMENT_TREE_HPP
