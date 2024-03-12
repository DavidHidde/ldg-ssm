#ifndef COSINE_DISTANCE_HPP
#define COSINE_DISTANCE_HPP

#include <memory>

namespace ldg
{
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

        double norm = (*lhs).norm() * (*rhs).norm();
        return norm == 0. ? 0. : 1. - (*lhs).dot(*rhs) / norm;  // Completely empty vectors are likely aggregates of void cells, so set to 0
    }
}

#endif //COSINE_DISTANCE_HPP
