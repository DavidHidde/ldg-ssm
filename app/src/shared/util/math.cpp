#include "app/include/shared/util/math.hpp"

namespace shared
{
    /**
     * Get the ceiling power of 2 for a size_t equivalent to "ceil(log2(num))".
     *
     * @param num
     * @return
     */
    size_t ceilPowerOfTwo(size_t num)
    {
        --num;

        num |= num >> 1;
        num |= num >> 2;
        num |= num >> 4;
        num |= num >> 8;
        num |= num >> 16;

        return num + 1;
    }

    /**
     * @param row
     * @param col
     * @param num_cols
     * @return
     */
    size_t rowMajorIndex(size_t row, size_t col, size_t num_cols)
    {
        return col + row * num_cols;
    }

    /**
     * @param row
     * @param col
     * @param num_cols
     * @return
     */
    long rowMajorIndexLong(long row, long col, long num_cols)
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
    size_t ceilDivideByFactor(size_t num, float factor)
    {
        return size_t(std::ceil(float(num) / factor));
    }

    /**
     * Simple shorthand to divide by two and round up.
     *
     * @param num
     * @param power
     * @return
     */
    size_t ceilDivideByPowerTwo(size_t num, size_t power)
    {
        return ceilDivideByFactor(num, std::pow(2., power));
    }
}