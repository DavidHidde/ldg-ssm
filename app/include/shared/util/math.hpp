#ifndef LDG_CORE_MATH_HPP
#define LDG_CORE_MATH_HPP

#include <cstddef>
#include <cmath>

namespace shared
{
    size_t rowMajorIndex(size_t row, size_t col, size_t num_cols);

    size_t ceilDivideByFactor(size_t num, float factor);

    size_t ceilDivideByPowerTwo(size_t num, size_t power);
}

#endif //LDG_CORE_MATH_HPP
