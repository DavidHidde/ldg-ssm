#ifndef LDG_CORE_METHOD_HPP
#define LDG_CORE_METHOD_HPP

#include <functional>
#include <algorithm>
#include "app/include/shared/model/quad_assignment_tree.hpp"

namespace ssm
{
    template<typename DataType>
    void sort(
        shared::QuadAssignmentTree<DataType> quad_tree,
        std::function<float(DataType, DataType)> distance_function,
        size_t max_iterations
    );
}

#endif //LDG_CORE_METHOD_HPP
