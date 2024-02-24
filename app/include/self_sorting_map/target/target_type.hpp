#ifndef LDG_CORE_TARGET_TYPE_HPP
#define LDG_CORE_TARGET_TYPE_HPP

namespace ssm
{
    /**
     * Types of targets for the self-sorting map.
     */
    enum TargetType
    {
        HIERARCHY,                  // Just the parents in the hierarchy
        NEIGHBOURHOOD,              // Just the neighbourhood of parent aggregates
        HIERARCHY_NEIGHBOURHOOD     // Both the hierarchy and neighborhood components
    };
}

#endif //LDG_CORE_TARGET_TYPE_HPP
