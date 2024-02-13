#ifndef LDG_CORE_CELL_POSITION_HPP
#define LDG_CORE_CELL_POSITION_HPP

#include <cstddef>

namespace shared
{
    /**
     * Simple POD to indicate the position of a cell in the quad tree.
     */
    struct CellPosition {
        size_t height;
        size_t index;
    };
}

#endif //LDG_CORE_CELL_POSITION_HPP
