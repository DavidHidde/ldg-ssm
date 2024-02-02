#include "app/include/shared/util/tree_traversal/tree_walker.hpp"

namespace shared
{
    /**
     * @tparam DataType
     * @param index
     * @param height
     * @param quad_tree
     */
    template<typename DataType>
    TreeWalker<DataType>::TreeWalker(CellPosition &position, QuadAssignmentTree<DataType> &quad_tree):
        node(position),
        num_rows(quad_tree->getNumRows() * std::pow(2, -position.height)),
        num_cols(quad_tree->getNumCols() * std::pow(2, -position.height)),
        quad_tree(quad_tree)
    {
    }

    /**
     * Move up in the quad tree. Returns false if at the root.
     *
     * @tparam DataType
     * @return True if the walker is now set at the parent, else false.
     */
    template<typename DataType>
    bool TreeWalker<DataType>::moveUp()
    {
        if (node.height == quad_tree->getDepth() - 1) {
            return false;
        }

        ++node.height;
        node.index = getParentIndex();
        num_cols /= 2;
        num_rows /= 2;
        return true;
    }

    /**
     * Move down in the quad tree. Returns false if at a leaf.
     *
     * @param quadrant The quadrant of the child to move to.
     * @tparam DataType
     * @return True if the walker is now set at one of the children, else false.
     */
    template<typename DataType>
    bool TreeWalker<DataType>::moveDown(Quadrant quadrant)
    {
        if (node.height == 0) {
            return false;
        }

        auto child_indices = getChildrenIndices();
        --node.height;
        num_cols *= 2;
        num_rows *= 2;

        switch (quadrant) {
            case Quadrant::NORTH_WEST:
                node.index = child_indices[0];
                break;
            case Quadrant::NORTH_EAST:
                node.index = child_indices[1];
                break;
            case Quadrant::SOUTH_WEST:
                node.index = child_indices[2];
                break;
            case Quadrant::SOUTH_EAST:
                node.index = child_indices[3];
                break;
        }

        return true;
    }

    /**
     * Reposition the walker on a new place in the tree. It is your responsibility to make sure these are legal.
     *
     * @tparam DataType
     * @param index
     * @param height
     * @return
     */
    template<typename DataType>
    bool TreeWalker<DataType>::reposition(CellPosition &position)
    {
        node.index = position.index;
        node.height = position.height;
        num_rows = quad_tree->getNumRows() * std::pow(2, -position.height);
        num_cols = quad_tree->getNumCols() * std::pow(2, -position.height);
    }

    /**
     * Get the current node value. Returns nullptr if it's out of bounds
     *
     * @tparam DataType
     * @return The data of the current node if it exists, else nullptr.
     */
    template<typename DataType>
    DataType &TreeWalker<DataType>::getNode()
    {
        auto bounds = getHeightBounds(node.height);
        size_t computed_idx = bounds.first + node.index;
        return computed_idx < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[computed_idx]] : nullptr;
    }

    /**
     * Get the parent node if it exists.
     *
     * @tparam DataType
     * @return The data of the parent node if it exists, else nullptr.
     */
    template<typename DataType>
    DataType &TreeWalker<DataType>::getParent()
    {
        if (node.height == quad_tree->getDepth() - 1) {
            return nullptr;
        }

        size_t parent_idx = getParentIndex();
        auto bounds = getHeightBounds(node.height + 1);
        return parent_idx < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[parent_idx]] : nullptr;;
    }

    /**
     * Get the children of the nodes if they exist.
     *
     * @tparam DataType
     * @return The data of the child nodes if they exist, else nullptr per child the child does not exist.
     */
    template<typename DataType>
    std::array<DataType, 4> TreeWalker<DataType>::getChildren()
    {
        if (node.height == 0) {
            return { nullptr, nullptr, nullptr, nullptr };
        }

        auto child_indices = getChildrenIndices();
        auto bounds = getHeightBounds(node.height - 1);

        return {
            child_indices[0] < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[child_indices[0]]] : nullptr,
            child_indices[1] < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[child_indices[1]]] : nullptr,
            child_indices[2] < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[child_indices[2]]] : nullptr,
            child_indices[3] < bounds.second ? quad_tree->getData()[quad_tree->getAssignment()[child_indices[3]]] : nullptr,
        };
    }

    /**
     * Get the bounds of the data arrays for the set height.
     *
     * @param desired_height
     * @tparam DataType
     * @return [start, end) of the data array at the given height.
     */
    template<typename DataType>
    std::pair<size_t, size_t> TreeWalker<DataType>::getHeightBounds(size_t desired_height)
    {
        size_t offset = quad_tree->getNumRows() * quad_tree->getNumCols() * ((1. - std::pow(
            0.25,
            desired_height
        )) / (1. - 0.25)); // Geometric summation.
        return {
            offset,
            offset + num_rows * num_cols
        };
    }

    /**
     * Get the index of the parent of this node.
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t TreeWalker<DataType>::getParentIndex()
    {
        return (node.index % 2) / 2 + (node.index / (2 * num_cols)) * (num_cols / 2);
    }

    /**
     * Get the indices of the children of this node.
     *
     * @tparam DataType
     * @return Child indices of the node, in the order: [NW, NE, SW, SE]
     */
    template<typename DataType>
    std::array<size_t, 4> TreeWalker<DataType>::getChildrenIndices()
    {
        size_t index = (node.index % 2) * 2 + (node.index / num_cols) * num_cols * 4;
        return {
            index,                      // North-west
            index + 1,                  // North-east
            index + num_cols * 2,       // South-west
            index + num_cols * 2 + 1    // South-east
        };
    }
} // shared