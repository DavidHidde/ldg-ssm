#ifndef NEW_LDG_EXPORT_HPP
#define NEW_LDG_EXPORT_HPP

#include <string>
#include <functional>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "export_settings.hpp"
#include "app/include/program/export/image.hpp"
#include "app/include/ldg/util/metric/disparity.hpp"
#include "app/include/adapter/storage.hpp"
#include "app/include/program/input/input_configuration.hpp"

namespace program
{
    /**
     * Calculate the disparities, compress and save them and create and save an input config that points to it.
     *
     * @tparam VectorType
     * @param output_dir
     * @param file_name
     * @param quad_tree
     * @param distance_function
     * @return
     */
    template<typename VectorType>
    std::string createDisparityInputConfiguration(
        std::string output_dir,
        std::string file_name,
        ldg::QuadAssignmentTree<VectorType> &quad_tree,
        std::function<double(std::shared_ptr<VectorType>, std::shared_ptr<VectorType>)> distance_function
    ) {
        std::string disparity_file_name = file_name + "-disparity";
        auto disparities = computeDisparity(quad_tree, distance_function);
        adapter::saveAndCompressIterable(quad_tree, disparities, output_dir + disparity_file_name);

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

        ExportConfiguration export_configuration;
        export_configuration.visualization_config_path = settings.visualization_config_path;
        std::string assignment_file_name = settings.file_name + "-assignment";
        export_configuration.assignment_path = assignment_file_name + ".raw.bz2";
        adapter::saveAndCompressAssignment<VectorType>(quad_tree, settings.output_dir + assignment_file_name);

        if (settings.export_data) {
            std::string data_file_name = settings.output_dir + settings.file_name + "-data";
            // Save the data in some way, converting it to images and then compressing it.
        }
        if (settings.export_disparity) {
            export_configuration.disparity_config_path = createDisparityInputConfiguration(settings.output_dir, settings.file_name, quad_tree, distance_function);
            // At this point we have set everything so we perform the export
            export_configuration.toJSONFile(settings.output_dir + settings.file_name + "-config");
        }
    }
}

#endif //NEW_LDG_EXPORT_HPP
