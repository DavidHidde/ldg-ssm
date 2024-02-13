#ifndef LDG_CORE_DISTANCE_FUNCTIONS_HPP
#define LDG_CORE_DISTANCE_FUNCTIONS_HPP

#include "helper_volData/vec.h"
#include <memory>
#include <cmath>

/**
 * Get the magnitude of a vector
 *
 * @tparam DataType
 * @param vector
 * @return
 */
template<typename DataType>
float magnitude(DataType vector)
{
    return std::sqrt(dot(vector, vector));
}

namespace shared
{
    /**
     * Take the euclidean distance between two data items.
     *
     * @tparam DataType
     * @param lhs
     * @param rhs
     * @return 0 if the items are equal or if one of the items is nullptr.
     */
    template<typename DataType>
    float euclideanDistance(std::shared_ptr<DataType> const &lhs, std::shared_ptr<DataType> const &rhs)
    {
        if (lhs == nullptr || rhs == nullptr)
            return 0.;

        return magnitude(*lhs - *rhs);
    }

    /**
     * Take the cosine distance between two items.
     *
     * @tparam DataType
     * @param lhs
     * @param rhs
     * @return 0 if the items are equal or if one of the items is nullptr.
     */
    template<typename DataType>
    float cosineDistance(std::shared_ptr<DataType> const &lhs, std::shared_ptr<DataType> const &rhs)
    {
        if (lhs == nullptr || rhs == nullptr)
            return 0.;

        return 1. - dot(*lhs, *rhs) / (magnitude(*lhs) * magnitude(*rhs));
    }
}

#endif //LDG_CORE_DISTANCE_FUNCTIONS_HPP
