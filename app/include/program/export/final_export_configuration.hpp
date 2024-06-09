#ifndef NEW_LDG_CONFIGURATION_HPP
#define NEW_LDG_CONFIGURATION_HPP

#include <cstddef>
#include <utility>
#include <string>
#include <array>
#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

namespace program
{
    /**
     * Simple configuration for export which points to an assignment, its disparity and its visualization
     */
    struct FinalExportConfiguration
    {
        std::string assignment_path;
        std::string disparity_config_path;
        std::string visualization_config_path;

        void toJSONFile(std::string file_name);

    private:
        const std::string KEYWORD_ASSIGNMENT = "assignment";
        const std::string KEYWORD_DISPARITY_CONFIG = "disparity_config";
        const std::string KEYWORD_VISUALIZATION_CONFIG = "visualization_config";
    };

    /**
     * Save the config in a JSON file. Will append .json to the filename.
     * @param file_name
     */
    void FinalExportConfiguration::toJSONFile(std::string file_name)
    {
        JSON json;
        json[KEYWORD_ASSIGNMENT] = assignment_path;
        json[KEYWORD_DISPARITY_CONFIG] = disparity_config_path;
        json[KEYWORD_VISUALIZATION_CONFIG] = visualization_config_path;

        std::ofstream output_stream(file_name + ".json");
        output_stream << std::setw(4) << json << std::endl;
    }
} // program

#endif //NEW_LDG_CONFIGURATION_HPP
