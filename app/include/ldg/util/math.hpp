#ifndef LDG_CORE_MATH_HPP
#define LDG_CORE_MATH_HPP

#include <cstddef>
#include <cmath>
#include <Eigen/Dense>

namespace ldg
{
    /**
     * @param row
     * @param col
     * @param num_cols
     * @return
     */
    inline size_t rowMajorIndex(size_t row, size_t col, size_t num_cols)
    {
        return col + row * num_cols;
    }

    /**
     * Simple shorthand to divide by a factor and round up.
     *
     * @param num
     * @param factor
     * @return
     */
    inline size_t ceilDivideByFactor(size_t num, float factor)
    {
        return static_cast<size_t>(std::ceil(static_cast<float>(num) / factor));
    }

    /**
     * Simple shorthand to divide by two and round up.
     *
     * @param num
     * @param power
     * @return
     */
    inline size_t ceilDivideByPowerTwo(size_t num, size_t power)
    {
        return ceilDivideByFactor(num, std::pow(2., power));
    }

    /**
     * Aggregate multiple vectors into one, ignoring null pointers and dividing by the number of elements.
     *
     * @tparam VectorType
     * @param vectors
     * @param vector_num_elements
     * @return
     */
    template<typename VectorType>
    VectorType aggregate(std::vector<std::shared_ptr<VectorType>> &vectors, size_t vector_num_elements)
    {
        double count = 0.;
        VectorType aggregate = VectorType::Zero(vector_num_elements);

        for (auto vector_ptr : vectors) {
            if (vector_ptr != nullptr) {
                aggregate += *vector_ptr;
                ++count;
            }
        }

        return aggregate / std::max(1., count);
    }

    /**
     * Find the value with the minimum distance to all other vectors.
     * Assumes a symmetric distance function.
     *
     * @tparam VectorType
     * @param vectors
     * @param distance_function
     * @return
     */
    template<typename VectorType>
    VectorType findMinimum(
        std::vector<std::shared_ptr<VectorType>> &vectors,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    ) {
        double min_distance = std::numeric_limits<double>::max();
        VectorType result;
        std::vector<double> distances(vectors.size(), 0.);
        for (size_t idx = 0; idx < vectors.size(); ++idx) {
            if (vectors[idx] == nullptr)
                continue;

            for (size_t compare_idx = idx + 1; compare_idx < vectors.size(); ++compare_idx) {
                double distance = distance_function(vectors[idx], vectors[compare_idx]);
                distances[idx] += distance;
                distances[compare_idx] += distance;
            }

            // Check if total distance is smaller and accept if it is
            if (min_distance > distances[idx]) {
                result = *vectors[idx];
                min_distance = distances[idx];
            }
        }

        return result;
    }

    /**
     * Aggregate multiple vectors into one, ignoring null pointers and dividing by the number of elements.
     * Note that we assume that the default constructor of the template type initializes to 0.
     *
     * @tparam VectorType
     * @param vectors
     * @param num_elements
     * @return
     */
    template<typename VectorType>
    VectorType aggregate(std::vector<std::shared_ptr<VectorType>> &&vectors, size_t num_elements)
    {
        double count = 0.;
        VectorType aggregate = VectorType::Zero(num_elements);

        for (auto vector_ptr : vectors) {
            if (vector_ptr != nullptr) {
                aggregate += *vector_ptr;
                ++count;
            }
        }

        return aggregate / std::max(1., count);
    }
}

#endif //LDG_CORE_MATH_HPP
