#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <random>
#include "helper_volData/vec.h"
#include "app/include/shared/util/math.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/distance_functions.hpp"
#include "app/include/shared/tree_functions.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/shared/util/metric/hierarchy_neighborhood_distance.hpp"

/**
 * Generate some random RGB data in the range [0, 255], already in the quad tree structure.
 *
 * @param num_rows
 * @param num_cols
 * @return
 */
std::shared_ptr<std::vector<std::shared_ptr<V3<float>>>> generateRandomColorData(size_t num_rows, size_t num_cols)
{
    // Generate quad tree structure space, initialized to an empty shared ptr
    size_t num_elements = num_rows * num_cols;
    size_t size = num_elements;
    while (num_rows > 1 && num_cols > 1) {
        num_cols = shared::ceilDivideByFactor(num_cols, 2.);
        num_rows = shared::ceilDivideByFactor(num_rows, 2.);
        size += num_cols * num_rows;
    }
    auto data = std::make_shared<std::vector<std::shared_ptr<V3<float>>>>(size);

    // Fill first cells with data
    for (size_t idx = 0; idx < size; ++idx) {
        if (idx < num_elements) {
            float factor = (float(idx) / float(num_elements)) * 255.;
            (*data)[idx] = std::make_shared<V3<float>>(V3<float>{ factor, factor, factor });
        } else {
            (*data)[idx] = std::make_shared<V3<float>>(V3<float>{});
        }
    }

    return data;
}

/**
 * Create a random assignment for given dimensions.
 *
 * @param num_rows
 * @param num_cols
 * @return
 */
std::shared_ptr<std::vector<size_t>> createRandomAssignment(size_t size, size_t num_rows, size_t num_cols)
{
    auto assignment = std::make_shared<std::vector<size_t>>(size);

    // Generate quad tree structure space, where every height starts at 0 again
    size_t offset = 0;
    while (num_rows > 1 && num_cols > 1) {
        size_t end = offset + num_cols * num_rows;
        for (size_t idx = offset; idx < end; ++idx) {
            (*assignment)[idx] = idx;
        }
        std::shuffle((*assignment).begin() + offset, (*assignment).begin() + end, std::mt19937());
        offset += num_cols * num_rows;
        num_cols = shared::ceilDivideByFactor(num_cols, 2.);
        num_rows = shared::ceilDivideByFactor(num_rows, 2.);
    }
    (*assignment)[size - 1] = size - 1; // Fix last element

    return assignment;
}

int main(int argc, const char **argv)
{
    // Runtime test parameters
    size_t n_rows = 32;
    size_t n_cols = 32;
    size_t max_iterations = 1000;

    size_t depth = std::ceil(std::log2(std::max(n_cols, n_rows))) + 1;
    auto data = generateRandomColorData(n_rows, n_cols);
    auto assignment = createRandomAssignment((*data).size(), n_rows, n_cols);
    shared::QuadAssignmentTree<V3<float>> quad_tree{ data, assignment, n_rows, n_cols, depth };
    auto tree_data = *quad_tree.getData();

    std::function<float(std::shared_ptr<V3<float>>, std::shared_ptr<V3<float>>)> distance_function = shared::euclideanDistance<V3<float>>;
    std::cout << "Start HND: " << shared::computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree) << "\n\n";
    ssm::sort(quad_tree, distance_function, max_iterations);
    std::cout << "After sorting HND: " << shared::computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree) << '\n';

    return 0;
}