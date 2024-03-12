#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "app/include/ldg/util/math.hpp"
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "app/include/self_sorting_map/method.hpp"
#include "app/include/ldg/util/image.hpp"
#include "app/include/ldg/util/metric/hierarchy_neighborhood_distance.hpp"
#include "app/include/adapter/data.hpp"
#include "app/include/adapter/storage.hpp"
#include "app/include/ldg/util/metric/cosine_distance.hpp"
#include "app/include/ldg/util/metric/euclidean_distance.hpp"

/**
 * Generate some random RGB data in the range [0, 255], already in the quad tree structure.
 *
 * @param num_rows
 * @param num_cols
 * @return
 */
std::vector<std::shared_ptr<Eigen::VectorXd>> generateRandomColorData(size_t num_rows, size_t num_cols)
{
    // Generate quad tree structure space, initialized to an empty shared ptr
    size_t num_elements = num_rows * num_cols;
    size_t size = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
    auto data = std::vector<std::shared_ptr<Eigen::VectorXd>>(size, std::make_shared<Eigen::VectorXd>(Eigen::Vector3d::Zero(3)));

    // Fill first cells with data
    for (size_t idx = 0; idx < size; ++idx) {
        if (idx < num_elements) {
            size_t x = idx % num_cols;
            size_t y = idx / num_cols;
            double r = x * (255. / (num_cols - 1));
            double g = (x + y) * (255. / (num_cols + num_rows - 2));
            double b = y * (255. / (num_rows - 1));
            data[idx] = std::make_shared<Eigen::VectorXd>(Eigen::Vector3d{ std::round(r), std::round(g), std::round(b) });
        } else {
            data[idx] = std::make_shared<Eigen::VectorXd>(Eigen::Vector3d::Zero(3));
        }
    }

    return data;
}

int main(int argc, const char **argv)
{
    bool use_synthetic_data = false;
    clock_t start = clock();

    // Forward declare all relevant variables.
    size_t n_rows;
    size_t n_cols;
    size_t max_iterations;
    double minimal_dist_change_percent;
    std::vector<ssm::TargetType> target_types;

    std::vector<std::shared_ptr<Eigen::VectorXd>> data;
    std::vector<size_t> assignment;

    size_t element_len;
    size_t num_elements;

    std::function<double(
        std::shared_ptr<Eigen::VectorXd>,
        std::shared_ptr<Eigen::VectorXd>
    )> distance_function;
    std::function<void(
        ldg::QuadAssignmentTree<Eigen::VectorXd> &,
        std::string
    )> checkpoint_function;

    if (use_synthetic_data) {
        n_rows = 128;
        n_cols = 128;
        max_iterations = 1000;
        minimal_dist_change_percent = 0.0000001;
        target_types = std::vector{ ssm::PARTITION_NEIGHBOURHOOD };

        element_len = 3;
        num_elements = n_rows * n_cols;

        data = generateRandomColorData(n_rows, n_cols);
        assignment = ldg::createAssignment(data.size(), n_rows, n_cols);

        distance_function = ldg::euclideanDistance<Eigen::VectorXd>;
        checkpoint_function = ldg::saveQuadTreeRGBImages<Eigen::VectorXd>;
    } else {
        max_iterations = 100;
        minimal_dist_change_percent = 0.000001;
        target_types = std::vector{ ssm::PARTITION_NEIGHBOURHOOD };

        auto [loaded_data, dims, loaded_element_len, loaded_num_elements] = adapter::loadData(
            "/usr/data/input/stock1.config"
        );
        n_rows = dims.first;
        n_cols = dims.second;
        data = loaded_data;
        element_len = loaded_element_len;
        num_elements = loaded_num_elements;
        assignment = adapter::readCompressedAssignment(
            "/usr/data/output/stock1/qtLeafAssignment.raw.bz2",
            n_rows,
            n_cols,
            num_elements
        );
        // assignment = ldg::createAssignment(data.size(), n_rows, n_cols);

        distance_function = ldg::cosineDistance<Eigen::VectorXd>;
        checkpoint_function = adapter::saveAndCompressAssignment<Eigen::VectorXd>;
    }

    // Data initialization
    size_t depth = std::ceil(std::log2(std::max(n_cols, n_rows))) + 1;
    ldg::QuadAssignmentTree<Eigen::VectorXd> quad_tree{ data, assignment, n_rows, n_cols, depth, num_elements, element_len };
    ldg::assertUniqueAssignment(quad_tree);

    // Actual sorting
    std::cout << "Pre-randomization HND: " << ldg::computeHierarchyNeighborhoodDistance(0, distance_function, quad_tree) << "\n";
    ldg::randomizeAssignment(quad_tree, 42);
    ssm::sort(quad_tree, distance_function, checkpoint_function, max_iterations, minimal_dist_change_percent, target_types);
    ldg::assertUniqueAssignment(quad_tree);

    clock_t stop = clock();
    double elapsed = (double) (stop - start) / CLOCKS_PER_SEC;
    printf("\nTime elapsed: %.5f\n", elapsed);
    return 0;
}