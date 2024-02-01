#ifndef LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
#define LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP

#include <functional>
#include "app/include/shared/model/quad_assignment_tree.hpp"

namespace shared
{
    const float EMPTY_HND_SCORE = -1.;

    template<typename DataType>
    float computeHierarchyNeighborhoodDistance(
        size_t height,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source,
        QuadAssignmentTree<float> &cache
    );

    template<typename DataType>
    float computeHierarchyNeighborhoodDistanceForCell(
        CellPosition &position,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source,
        QuadAssignmentTree<float> &cache
    );

    template<typename DataType>
    QuadAssignmentTree<float> generateCacheTree(QuadAssignmentTree<DataType> &source);
}

#endif //LDG_CORE_HIERARCHY_NEIGHBORHOOD_DISTANCE_HPP
