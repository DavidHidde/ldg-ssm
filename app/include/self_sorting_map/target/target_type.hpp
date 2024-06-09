#ifndef LDG_CORE_TARGET_TYPE_HPP
#define LDG_CORE_TARGET_TYPE_HPP

namespace ssm
{
    /**
     * Types of targets for the self-sorting map.
     * 4-connected variants also aggregate the 4-connected neighbourhood into the target.
     */
    enum TargetType
    {
        AGGREGATE_HIERARCHY,            // Aggregate parents in the hierarchy
        HIGHEST_PARENT_HIERARCHY,       // Highest unique parent per partition
        AGGREGATE_HIERARCHY_4C,         // 4-connected variant of AGGREGATE_HIERARCHY
        HIGHEST_PARENT_HIERARCHY_4C,    // 4-connected variant of HIGHEST_PARENT_HIERARCHY
        PARTITION_NEIGHBOURHOOD,        // Neighbourhood of parent aggregates
        CELL_NEIGHBOURHOOD,             // Direct neighbourhood per cell.
    };
}

#endif //LDG_CORE_TARGET_TYPE_HPP
