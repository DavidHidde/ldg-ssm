#ifndef EUCLIDEAN_DISTANCE_HPP
#define EUCLIDEAN_DISTANCE_HPP

#include <memory>

namespace ldg
{
    /**
     * Take the vector-length normalized euclidean distance between two data items.
     *
     * @tparam VectorType
     * @param lhs
     * @param rhs
     * @return 0 if the items are equal or if one of the items is nullptr.
     */
    template<typename VectorType>
    double normalizedEuclideanDistance(std::shared_ptr<VectorType> const &lhs, std::shared_ptr<VectorType> const &rhs)
    {
        if (lhs == nullptr || rhs == nullptr)
            return 0.;

//        return (*lhs - *rhs).norm() / std::sqrt((*lhs).size());
        return std::sqrt((*lhs - *rhs).squaredNorm() / static_cast<double>((*lhs).size()));
    }
}

#endif //EUCLIDEAN_DISTANCE_HPP
