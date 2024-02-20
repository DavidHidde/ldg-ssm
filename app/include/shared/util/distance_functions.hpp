#ifndef LDG_CORE_DISTANCE_FUNCTIONS_HPP
#define LDG_CORE_DISTANCE_FUNCTIONS_HPP

#include "helper_volData/vec.h"
#include <memory>
#include <cmath>

/**
 * Get the magnitude of a vector
 *
 * @tparam VectorType
 * @param vector
 * @return
 */
template<typename VectorType>
double magnitude(VectorType vector)
{
    return std::sqrt(dot(vector, vector));
}

namespace shared
{
    /**
     * Take the euclidean distance between two data items.
     *
     * @tparam VectorType
     * @param lhs
     * @param rhs
     * @return 0 if the items are equal or if one of the items is nullptr.
     */
    template<typename VectorType>
    double euclideanDistance(std::shared_ptr<VectorType> const &lhs, std::shared_ptr<VectorType> const &rhs)
    {
        if (lhs == nullptr || rhs == nullptr)
            return 0.;

        return magnitude(*lhs - *rhs);
    }

    /**
     * Take the cosine distance between two items.
     *
     * @tparam VectorType
     * @param lhs
     * @param rhs
     * @return 0 if the items are equal or if one of the items is nullptr.
     */
    template<typename VectorType>
    double cosineDistance(std::shared_ptr<VectorType> const &lhs, std::shared_ptr<VectorType> const &rhs)
    {
        if (lhs == nullptr || rhs == nullptr)
            return 0.;

        return 1. - dot(*lhs, *rhs) / (magnitude(*lhs) * magnitude(*rhs));
    }
}

#endif //LDG_CORE_DISTANCE_FUNCTIONS_HPP
