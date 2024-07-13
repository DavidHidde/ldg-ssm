#ifndef SORT_OPTIONS_HPP
#define SORT_OPTIONS_HPP

#include "app/include/self_sorting_map/target/target_type.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"

#include <functional>
#include <memory>
#include <string>

namespace program
{
    /**
     * Options that should be used for sorting. These essentially serve as input for ssm::sort next to the data
     * @tparam VectorType
     */
    template<typename VectorType>
    struct SortOptions
    {
        bool randomize_assignment;
        bool use_partition_swaps;
        bool ssm_mode;
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function;
    };
}

#endif //SORT_OPTIONS_HPP