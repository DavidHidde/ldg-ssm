#ifndef LDG_CORE_DISTANCE_FUNCTIONS_HPP
#define LDG_CORE_DISTANCE_FUNCTIONS_HPP

#include <memory>

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

        return (*lhs - *rhs).norm();
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

        return 1. - (*lhs).dot(*rhs) / ((*lhs).norm() * (*rhs).norm());
    }
}

#endif //LDG_CORE_DISTANCE_FUNCTIONS_HPP
