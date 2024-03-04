#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "app/include/shared/util/math.hpp"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/distance_functions.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/shared/util/image.hpp"
#include "app/include/shared/util/metric/hierarchy_neighborhood_distance.hpp"

/**
 * Generate some random RGB data in the range [0, 255], already in the quad tree structure.
 *
 * @param num_rows
 * @param num_cols
 * @return
 */
std::vector<std::shared_ptr<Eigen::Vector3d>> generateRandomColorData(size_t num_rows, size_t num_cols)
{
    // Generate quad tree structure space, initialized to an empty shared ptr
    size_t num_elements = num_rows * num_cols;
    size_t size = num_elements;
    size_t new_num_cols = num_cols;
    size_t new_num_rows = num_rows;
    while (new_num_rows > 1 && new_num_cols > 1) {
        new_num_cols = shared::ceilDivideByFactor(new_num_cols, 2.);
        new_num_rows = shared::ceilDivideByFactor(new_num_rows, 2.);
        size += new_num_cols * new_num_rows;
    }
    auto data = std::vector<std::shared_ptr<Eigen::Vector3d>>(size, std::make_shared<Eigen::Vector3d>(Eigen::Vector3d::Zero()));

    // Fill first cells with data
    for (size_t idx = 0; idx < size; ++idx) {
        if (idx < num_elements) {
            size_t x = idx % num_cols;
            size_t y = idx / num_cols;
            double r = x * (255. / (num_cols - 1));
            double g = (x + y) * (255. / (num_cols + num_rows - 2));
            double b = y * (255. / (num_rows - 1));
            data[idx] = std::make_shared<Eigen::Vector3d>(Eigen::Vector3d{ std::round(r), std::round(g), std::round(b) });
        } else {
            data[idx] = std::make_shared<Eigen::Vector3d>(Eigen::Vector3d::Zero());
        }
    }

    return data;
}

int main(int argc, const char **argv)
{
    clock_t start = clock();
    // Runtime test parameters
    size_t n_rows = 1024;
    size_t n_cols = 1024;
    size_t max_iterations = 10000;
    double minimal_dist_change_percent = 0.000001;
    std::vector<ssm::TargetType> target_types{ ssm::PARTITION_NEIGHBOURHOOD };

    // Data initialization
    size_t depth = std::ceil(std::log2(std::max(n_cols, n_rows))) + 1;
    auto data = generateRandomColorData(n_rows, n_cols);
    auto assignment = shared::createAssignment(data.size(), n_rows, n_cols);
    shared::QuadAssignmentTree<Eigen::Vector3d> quad_tree{ data, assignment, n_rows, n_cols, depth };

    // Functions
    std::function<double(
        std::shared_ptr<Eigen::Vector3d>,
        std::shared_ptr<Eigen::Vector3d>
    )> distance_function = shared::euclideanDistance<Eigen::Vector3d>;
    std::function<void(
        shared::QuadAssignmentTree<Eigen::Vector3d> &,
        std::string const
    )> checkpoint_function = shared::saveQuadTreeRGBImages<Eigen::Vector3d>;

    // Actual sorting
    std::cout << "Fully sorted HND: " << computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree) << "\n";
    randomizeAssignment(quad_tree, 42);
    ssm::sort(quad_tree, distance_function, checkpoint_function, max_iterations, minimal_dist_change_percent, target_types);

    clock_t stop = clock();
    double elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
    printf("\nTime elapsed: %.5f\n", elapsed);
    return 0;
}