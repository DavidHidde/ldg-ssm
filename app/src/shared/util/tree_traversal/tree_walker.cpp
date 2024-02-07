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
        num_rows(ceilDivideByPowerTwo(quad_tree->getNumRows(), position.height)),
        num_cols(ceilDivideByPowerTwo(quad_tree->getNumCols(), position.height)),
        quad_tree(quad_tree)
    {
    }

    /**
     * @tparam DataType
     * @param position
     * @param num_rows
     * @param num_cols
     * @param quad_tree
     */
    template<typename DataType>
    TreeWalker<DataType>::TreeWalker(
        CellPosition &position,
        size_t num_rows,
        size_t num_cols,
        QuadAssignmentTree<DataType> &quad_tree
    ):
        node(position),
        num_rows(num_rows),
        num_cols(num_cols),
        quad_tree(quad_tree)
    {}

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
        num_cols = ceilDivideByFactor(num_cols, 2.);
        num_rows = ceilDivideByFactor(num_rows, 2.);
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
        int new_index = -1;
        switch (quadrant) {
            case Quadrant::NORTH_WEST:
                new_index = child_indices[0];
                break;
            case Quadrant::NORTH_EAST:
                new_index = child_indices[1];
                break;
            case Quadrant::SOUTH_WEST:
                new_index = child_indices[2];
                break;
            case Quadrant::SOUTH_EAST:
                new_index = child_indices[3];
                break;
        }

        // If the child does not exist, we can't descend.
        if (new_index < 0)
            return false;

        node.index = size_t(new_index);
        --node.height;
        // Tree is build bottom-up, so we need to recalculate this.
        float factor = std::pow(2, node.height);
        num_cols = ceilDivideByFactor(quad_tree->getNumCols(), factor);
        num_rows = ceilDivideByFactor(quad_tree->getNumRows(), factor);

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
        float factor = std::pow(2, position.height);
        num_rows = ceilDivideByFactor(quad_tree->getNumRows(), factor);
        num_cols = ceilDivideByFactor(quad_tree->getNumCols(), factor);
    }

    /**
     * Get the current node value. Returns nullptr if it's out of bounds
     *
     * @tparam DataType
     * @return The data of the current node if it exists, else nullptr.
     */
    template<typename DataType>
    DataType &TreeWalker<DataType>::getNodeValue()
    {
        return quad_tree->getValue(node);
    }

    /**
     * Get the parent node if it exists.
     *
     * @tparam DataType
     * @return The data of the parent node if it exists, else nullptr.
     */
    template<typename DataType>
    DataType &TreeWalker<DataType>::getParentValue()
    {
        if (node.height == quad_tree->getDepth() - 1) {
            return nullptr;
        }

        size_t parent_idx = getParentIndex();
        return quad_tree->getValue(CellPosition{ node.height + 1, parent_idx });
    }

    /**
     * Get the children of the nodes if they exist.
     *
     * @tparam DataType
     * @return The data of the child nodes if they exist, else nullptr per child the child does not exist.
     */
    template<typename DataType>
    std::array<DataType, 4> TreeWalker<DataType>::getChildrenValues()
    {
        if (node.height == 0) {
            return { nullptr, nullptr, nullptr, nullptr };
        }

        auto child_indices = getChildrenIndices();
        return {
            child_indices[0] >= 0 ? quad_tree->getValue(size_t(child_indices[0])) : nullptr,
            child_indices[1] >= 0 ? quad_tree->getValue(size_t(child_indices[1])) : nullptr,
            child_indices[2] >= 0 ? quad_tree->getValue(size_t(child_indices[2])) : nullptr,
            child_indices[3] >= 0 ? quad_tree->getValue(size_t(child_indices[3])) : nullptr
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
        return rowMajorIndex((node.index % num_cols) / 2, node.index / (2 * num_cols), ceilDivideByFactor(num_cols, 2.));
    }

    /**
     * Get the indices of the children of this node.
     * Because the quad tree is build from the bottom up and hence does not always split up into 4, we need to check
     * and also return nullptr if a child does not exist.
     *
     * @tparam DataType
     * @return Child indices of the node, in the order: [NW, NE, SW, SE]. If it does not exist, -1 is returned.
     */
    template<typename DataType>
    std::array<int, 4> TreeWalker<DataType>::getChildrenIndices()
    {
        float factor = std::pow(2, node.height);
        int new_num_cols = ceilDivideByFactor(quad_tree->getNumCols(), factor);
        int new_num_rows = ceilDivideByFactor(quad_tree->getNumRows(), factor);

        int new_row = (node.index / num_cols) * 2;
        int new_col = (node.index % num_cols) * 2;

        int index = int(rowMajorIndex(new_row, new_col, new_num_cols));
        return {
            index,                                                                                      // North-west
            new_col + 1 < new_num_cols ? index + 1 : -1,                                                // North-east
            new_row + 1 < new_num_rows ? index + new_num_cols : -1,                                     // South-west
            new_col + 1 < new_num_cols && new_row + 1 < new_num_rows ? index + new_num_cols + 1 : -1    // South-east
        };
    }

    /**
     * Get an iterator over all leaves below this node.
     *
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    RowMajorIterator<DataType> TreeWalker<DataType>::getLeaves()
    {
        auto bounds = quad_tree->getLeafBounds(node);
        auto start_end = bounds.first;
        auto dimensions = bounds.second;

        return {
            CellPosition{
                0,
                start_end.first
            },
            quad_tree,
            start_end.first,
            quad_tree->getNumRows(),
            quad_tree->getNumCols(),
            dimensions.first,
            dimensions.second
        };
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t TreeWalker<DataType>::getNumRows() const
    {
        return num_rows;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    size_t TreeWalker<DataType>::getNumCols() const
    {
        return num_cols;
    }

    /**
     * @tparam DataType
     * @return
     */
    template<typename DataType>
    CellPosition &TreeWalker<DataType>::getNode()
    {
        return node;
    }
} // shared