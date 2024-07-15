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
        size_t max_iterations;              // Maximum number of iterations before the SSM should move to the next height.
        double distance_threshold;          // Minimum ratio of distance that should be changed before the SSM should move to the next height.
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function;

        bool randomize_assignment;
        bool ssm_mode;
    };
}

#endif //SORT_OPTIONS_HPP