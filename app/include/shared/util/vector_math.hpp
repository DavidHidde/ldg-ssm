#ifndef LDG_CORE_VECTOR_MATH_HPP
#define LDG_CORE_VECTOR_MATH_HPP

#include <cmath>
#include <memory>
#include <vector>
#include <Eigen/Dense>

namespace shared
{
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
        VectorType aggregate = VectorType::Zero();

        for (auto vector_ptr : vectors) {
            if (vector_ptr != nullptr) {
                aggregate += *vector_ptr;
                ++count;
            }
        }

        return aggregate / count;
    }

    /**
     * Aggregate multiple vectors into one, ignoring null pointers and dividing by the total weight.
     * Note that we assume that the default constructor of the template type initializes to 0.
     *
     * @tparam VectorType
     * @param vectors
     * @param weights A vector of weights per vector. Assumed to be the same length as vectors.
     * @return
     */
    template<typename VectorType>
    VectorType aggregate(std::vector<std::shared_ptr<VectorType>> &vectors, std::vector<double> &weights)
    {
        double total_weight = 0.;
        VectorType aggregate = VectorType::Zero();

        for (size_t idx = 0; idx < vectors.size(); ++idx) {
            if (vectors[idx] != nullptr) {
                aggregate += weights[idx] * (*vectors[idx]);
                total_weight += weights[idx];
            }
        }

        return aggregate / total_weight;
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
        VectorType aggregate = VectorType::Zero();

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
