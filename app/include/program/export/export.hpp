#ifndef NEW_LDG_EXPORT_HPP
#define NEW_LDG_EXPORT_HPP

#include <string>
#include <functional>
#include <set>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "export_settings.hpp"
#include "app/include/program/export/image.hpp"
#include "app/include/ldg/util/metric/disparity.hpp"
#include "app/include/adapter/storage.hpp"
#include "app/include/program/input/input_configuration.hpp"

namespace program
{
    /**
     * Export the raw data as a .raw file with a JSON configuration.
     * TODO: compress the assignment such that void cells are not exported.
     *
     * @tparam VectorType
     * @param output_dir
     * @param file_name
     * @param has_existing_visualization
     * @param quad_tree
     * @return
     */
    template<typename VectorType>
    std::string exportRawData(
        std::string output_dir,
        std::string file_name,
        ldg::QuadAssignmentTree<VectorType> &quad_tree
    ) {
        std::string data_file_name = file_name + "-visualization-data";

        // Copy and save the data. We skip void cells.
        size_t element_len = quad_tree.getDataElementLen();
        size_t num_elements = quad_tree.getData().size();
        std::vector<double> data_copy(num_elements * element_len, 0.);  // Save as doubles regardless of the type.
        for (size_t idx = 0; idx < num_elements; ++idx) {
            auto &data_ptr = quad_tree.getData()[idx];
            if (data_ptr != nullptr) {
                std::copy((*data_ptr).begin(), (*data_ptr).end(), data_copy.begin() + idx * element_len); // Assumes type is an Eigen Vector type
            }
        }
        helper::bzip_compress(data_copy, output_dir + data_file_name + ".raw.bz2");

        // Create the input config for the data
        InputConfiguration visualization_input_config;
        visualization_input_config.grid_dims = { quad_tree.getNumRows(), quad_tree.getNumRows() };
        visualization_input_config.type = InputType::VISUALIZATION;
        visualization_input_config.data_dims = quad_tree.getDataDims();
        visualization_input_config.num_elements  = quad_tree.getData().size();
        visualization_input_config.data_path = data_file_name + ".raw.bz2";

        visualization_input_config.toJSONFile(output_dir + data_file_name);
        return data_file_name + ".json";
    }

    /**
     * Export the assignment for the visualization. This assignment also includes the parents of the base grid.
     * If the visualization data is provided, then we need to map the parents to the closest child.
     *
     * @tparam VectorType
     * @param output_dir
     * @param file_name
     * @param has_existing_visualization
     * @param quad_tree
     * @param distance_function
     * @return Relative path to the generated assignment
     */
    template<typename VectorType>
    std::string exportVisualizationAssignment(
        std::string output_dir,
        std::string file_name,
        bool has_existing_visualization,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    ) {
        std::vector<int> assignment_copy(quad_tree.getAssignment().begin(), quad_tree.getAssignment().end());

        if (has_existing_visualization) {
            // Replace all parents with the child that is closest to them
            std::vector<double> min_distances(assignment_copy.size(), std::numeric_limits<double>().infinity());
            for (size_t idx = 0; idx < quad_tree.getNumRows() * quad_tree.getNumCols(); ++idx) {
                ldg::TreeWalker walker({ 0, idx }, quad_tree);
                auto base_value = walker.getNodeValue();
                std::set<int> previous_values;  // Use a set to avoid not replacing overwritten parents
                while (base_value != nullptr && walker.moveUp()) {
                    double distance = distance_function(base_value, walker.getNodeValue());
                    auto [array_range, dims] = quad_tree.getBounds(walker.getNode());
                    size_t parent_idx = array_range.first + walker.getNode().index;
                    if (previous_values.count(assignment_copy[parent_idx]) > 0 || distance < min_distances[parent_idx]) {
                        previous_values.insert(assignment_copy[parent_idx]);
                        min_distances[parent_idx] = distance;
                        assignment_copy[parent_idx] = assignment_copy[idx];
                    } else {
                        break;
                    }
                }
            }
        }

        // Set all void cells to -1.
        for (size_t idx = 0; idx < assignment_copy.size(); ++idx) {
            if (quad_tree.getData()[assignment_copy[idx]] == nullptr)
                assignment_copy[idx] = -1;
        }

        std::string assignment_file_name = file_name + "-visualization-assignment";
        helper::bzip_compress(assignment_copy, output_dir + assignment_file_name + ".raw.bz2");
        return assignment_file_name + ".raw.bz2";
    }

    /**
     * Calculate the disparities, compress and save them and create and save an input config that points to it.
     *
     * @tparam VectorType
     * @param output_dir
     * @param file_name
     * @param quad_tree
     * @param distance_function
     * @return Relative path to the generated config
     */
    template<typename VectorType>
    std::string exportDisparity(
        std::string output_dir,
        std::string file_name,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    ) {
        std::string disparity_file_name = file_name + "-disparity";
        auto disparities = computeDisparity(quad_tree, distance_function);
        helper::bzip_compress(disparities, output_dir + disparity_file_name + ".raw.bz2");

        // Create the input config for the saved disparity values
        InputConfiguration disparity_configuration;
        disparity_configuration.type = InputType::DATA;
        disparity_configuration.num_elements = disparities.size();
        disparity_configuration.data_path = disparity_file_name + ".raw.bz2";
        disparity_configuration.grid_dims = { quad_tree.getNumRows(), quad_tree.getNumCols() };
        disparity_configuration.data_dims = { 1, 1, 1 };

        disparity_configuration.toJSONFile(output_dir + disparity_file_name);
        return disparity_file_name + ".json";
    }

    /**
     * Export the quad tree to storage.
     * Based on the export settings, this function either saves an RGB image, just the assignment or a configuration.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param distance_function
     * @param settings
     */
    template<typename VectorType>
    void exportQuadTree(
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function,
        ExportSettings &settings
    ) {
        if (settings.log_only) {
            return;
        }
        if (settings.debug) {
            return saveQuadTreeRGBImages<VectorType>(quad_tree, settings.output_dir + settings.file_name);
        }

        std::string assignment_file_name = settings.file_name + "-assignment";
        adapter::saveAndCompressAssignment<VectorType>(quad_tree, settings.output_dir + assignment_file_name);

        if (settings.export_data) {
            settings.visualization_config_path = exportRawData(settings.output_dir, settings.file_name, quad_tree);
        }
        if (settings.export_visualization) {
            FinalExportConfiguration export_configuration;
            export_configuration.visualization_config_path = settings.visualization_config_path;
            export_configuration.assignment_path = exportVisualizationAssignment(settings.output_dir, settings.file_name, !settings.export_data, quad_tree, distance_function);
            export_configuration.disparity_config_path = exportDisparity(settings.output_dir, settings.file_name, quad_tree, distance_function);

            // At this point we have set everything so we perform the export
            export_configuration.toJSONFile(settings.output_dir + settings.file_name + "-config");
        }
    }
}

#endif //NEW_LDG_EXPORT_HPP
