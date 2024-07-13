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
        HIGHEST_PARENT_HIERARCHY,       // Highest unique parent per partition
        PARTITION_NEIGHBOURHOOD         // Neighbourhood of parent aggregates
    };
}

#endif //LDG_CORE_TARGET_TYPE_HPP
