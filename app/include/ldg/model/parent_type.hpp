#ifndef LDG_SSM_PARENT_TYPE_HPP
#define LDG_SSM_PARENT_TYPE_HPP

namespace ldg
{
    /**
     * Type of aggregation the quad tree should use for parents.
     */
    enum ParentType {
        NORMALIZED_AVERAGE,
        MINIMUM_CHILD
    };
}

#endif //LDG_SSM_PARENT_TYPE_HPP
