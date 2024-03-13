#ifndef DISTANCE_FUNCTION_TYPES_HPP
#define DISTANCE_FUNCTION_TYPES_HPP

#include "cosine_distance.hpp"
#include "euclidean_distance.hpp"

#include <cassert>
#include <functional>
#include <memory>

namespace ldg
{
    /**
     * Types of supported distance functions.
     */
    enum DistanceFunctionType
    {
        EUCLIDEAN_DISTANCE,
        COSINE_SIMILARITY
    };

    /**
     * Map the distance function type to its actual function.
     *
     * @tparam VectorType
     * @param type
     * @return
     */
    template<typename VectorType>
    std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> mapFunctionTypeToFunction(DistanceFunctionType type)
    {
        switch (type) {
            case EUCLIDEAN_DISTANCE:
                return euclideanDistance<VectorType>;
            case COSINE_SIMILARITY:
                return cosineDistance<VectorType>;
            default:
                assert(false);  // Should never happen
        }
    }
}

#endif //DISTANCE_FUNCTION_TYPES_HPP
