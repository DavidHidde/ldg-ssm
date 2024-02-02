#ifndef LDG_CORE_TREE_WALKER_HPP
#define LDG_CORE_TREE_WALKER_HPP

#include "app/include/shared/model/quad_assignment_tree.hpp"
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
        CellPosition node;
        size_t num_rows;
        size_t num_cols;

        QuadAssignmentTree<DataType> *quad_tree;

        std::pair<size_t, size_t> getHeightBounds(size_t desired_height);

        size_t getParentIndex();

        std::array<size_t, 4> getChildrenIndices();

    public:
        TreeWalker(CellPosition &position, QuadAssignmentTree<DataType> &quad_tree);

        bool moveUp();

        bool moveDown(Quadrant quadrant);

        bool reposition(CellPosition &position);

        DataType &getNode();

        DataType &getParent();

        std::array<DataType, 4> getChildren();
    };
} // shared

#endif //LDG_CORE_TREE_WALKER_HPP
