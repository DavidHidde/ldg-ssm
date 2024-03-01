#ifndef LDG_CORE_TARGET_TYPE_HPP
#define LDG_CORE_TARGET_TYPE_HPP

namespace ssm
{
    /**
     * Types of targets for the self-sorting map.
     */
    enum TargetType
    {
        AGGREGATE_HIERARCHY,        // Aggregate parents in the hierarchy
        HIGHEST_PARENT_HIERARCHY,   // Highest unique parent per partition
        PARTITION_NEIGHBOURHOOD,    // Neighbourhood of parent aggregates
        CELL_NEIGHBOURHOOD,         // Direct neighbourhood per cell.
    };
}

#endif //LDG_CORE_TARGET_TYPE_HPP
