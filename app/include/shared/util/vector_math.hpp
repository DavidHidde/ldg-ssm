#ifndef LDG_CORE_VECTOR_MATH_HPP
#define LDG_CORE_VECTOR_MATH_HPP

#include <cmath>
#include <memory>
#include <vector>

namespace shared
{
    /**
     * Get the magnitude of a vector.
     *
     * @tparam VectorType
     * @param vector
     * @return
     */
    template<typename VectorType>
    double magnitude(VectorType &vector)
    {
        return std::sqrt(dot(vector, vector));
    }

    /**
     * Get the magnitude of a vector.
     *
     * @tparam VectorType
     * @param vector
     * @return
     */
    template<typename VectorType>
    double magnitude(VectorType &&vector)
    {
        return std::sqrt(dot(vector, vector));
    }

    /**
     * Aggregate multiple vectors into one, ignoring null pointers and dividing by the number of elements.
     * Note that we assume that the default constructor of the template type initializes to 0.
     *
     * @tparam VectorType
     * @param vectors
     * @return
     */
    template<typename VectorType>
    VectorType aggregate(std::vector<std::shared_ptr<VectorType>> &vectors)
    {
        double count = 0.;
        VectorType aggregate;

        for (auto vector_ptr : vectors) {
            if (vector_ptr != nullptr) {
                aggregate += *vector_ptr;
                ++count;
            }
        }

        return aggregate / count;
    }

    /**
     * Aggregate multiple vectors into one, ignoring null pointers and dividing by the number of elements.
     * Note that we assume that the default constructor of the template type initializes to 0.
     *
     * @tparam VectorType
     * @param vectors
     * @return
     */
    template<typename VectorType>
    VectorType aggregate(std::vector<std::shared_ptr<VectorType>> &&vectors)
    {
        double count = 0.;
        VectorType aggregate;

        for (auto vector_ptr : vectors) {
            if (vector_ptr != nullptr) {
                aggregate += *vector_ptr;
                ++count;
            }
        }

        return aggregate / count;
    }
}

#endif //LDG_CORE_VECTOR_MATH_HPP
