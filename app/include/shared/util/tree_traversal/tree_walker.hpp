#ifndef LDG_CORE_TREE_WALKER_HPP
#define LDG_CORE_TREE_WALKER_HPP

#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/math.hpp"
#include "row_major_iterator.hpp"

#include <array>
#include <cmath>

namespace shared
{
    /**
     * Class for walking up and down the quad tree (classic quad tree traversal).
     * The state considers a parent of 4 child nodes. For leaf nodes, the children are empty. For the root, the parent is.
     *
     * @tparam DataType Type of the underlying data.
     */
    template<typename DataType>
    class TreeWalker
    {
        CellPosition node;  // The current node.
        size_t num_rows;    // Number of rows of the current height array.
        size_t num_cols;    // Number of columns of the current height array.

        QuadAssignmentTree<DataType> *quad_tree;

    public:
        TreeWalker(CellPosition &position, QuadAssignmentTree<DataType> &quad_tree);

        TreeWalker(CellPosition &position, size_t num_rows, size_t num_cols, QuadAssignmentTree<DataType> &quad_tree);

        bool moveUp();

        bool moveDown(Quadrant quadrant);

        bool reposition(CellPosition &position);

        DataType &getNodeValue();

        DataType &getParentValue();

        std::array<DataType, 4> getChildrenValues();

        size_t getParentIndex();

        std::array<int, 4> getChildrenIndices();

        RowMajorIterator<DataType> getLeaves();

        size_t getNumRows() const;

        size_t getNumCols() const;

        CellPosition &getNode();
    };
} // shared

#endif //LDG_CORE_TREE_WALKER_HPP
