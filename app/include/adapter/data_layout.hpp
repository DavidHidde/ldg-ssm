#ifndef LDG_CORE_DATA_LAYOUT_HPP
#define LDG_CORE_DATA_LAYOUT_HPP

#include <vector>
#include <cstddef>
#include <cmath>
#include "app/include/shared/util/math.hpp"

/*
 * These functions handle layout conversions between two types of layouts:
 * 1. Hierarchical
 * [ 0  1  4  5  ]
 * [ 2  3  6  7  ]
 * [ 8  9  12 13 ]
 * [ 10 11 14 15 ]
 *
 * 2. Row-major
 * [ 0  1  2  3  ]
 * [ 4  5  6  7  ]
 * [ 8  9  10 11 ]
 * [ 12 13 14 15 ]
 *
 * Where the way of converting between both layouts is determined by the row-major partition index over all heights.
 */

namespace adapter
{
    /**
     * Copy data from a hierarchical vector to a row-major vector.
     * Note that this overrides the data in destination and assumes destination can hold the data.
     *
     *
     * @tparam SourceDataType
     * @tparam DestinationDataType
     * @param source
     * @param destination
     * @param num_rows
     * @param num_cols
     */
    template<typename SourceDataType, typename DestinationDataType>
    void copyFromHierarchyToRowMajor(
        std::vector<SourceDataType> &source,
        std::vector<DestinationDataType> &destination,
        size_t num_rows,
        size_t num_cols
    )
    {
        size_t depth = std::ceil(std::log2(std::max(num_rows, num_cols))) - 1;
        size_t partition_length = std::pow(2, depth++);
        std::vector<size_t> partition_lengths;
        std::vector<size_t> partition_areas;
        partition_lengths.reserve(depth);
        partition_areas.reserve(depth);
        for (size_t idx = 0; idx < depth; ++idx, partition_length /= 2) {
            partition_lengths.push_back(partition_length);
            partition_areas.push_back(partition_length * partition_length);
        }

        for (size_t idx = 0; idx < source.size(); ++idx) {
            size_t x = 0;
            size_t y = 0;

            size_t idx_copy = idx;
            for (size_t size_idx = 0; size_idx < depth; ++size_idx) {
                size_t partition_idx = idx_copy / partition_areas[size_idx];
                x += (partition_idx % 2) * partition_lengths[size_idx];
                y += (partition_idx / 2) * partition_lengths[size_idx];
                idx_copy %= partition_areas[size_idx];
            }

            if (x < num_cols && y < num_rows) {
                destination[shared::rowMajorIndex(y, x, num_cols)] = source[idx];
            }
        }
    }

    /**
     * Copy data from a hierarchical vector to a row-major vector.
     * Note that this overrides the data in destination and assumes destination can hold the data.
     *
     *
     * @tparam SourceDataType
     * @tparam DestinationDataType
     * @param source
     * @param destination
     * @param num_rows
     * @param num_cols
     */
    template<typename SourceDataType, typename DestinationDataType>
    void copyFromRowMajorToHierarchy(
        std::vector<SourceDataType> &source,
        std::vector<DestinationDataType> &destination,
        size_t num_rows,
        size_t num_cols
    )
    {
        for (auto it = source.begin(); it != source.end(); ++it) {

        }
    }
}

#endif //LDG_CORE_DATA_LAYOUT_HPP
