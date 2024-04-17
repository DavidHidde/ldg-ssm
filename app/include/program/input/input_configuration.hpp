#ifndef NEW_LDG_INPUT_CONFIGURATION_HPP
#define NEW_LDG_INPUT_CONFIGURATION_HPP

#include <utility>
#include <cstddef>
#include <array>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "input_type.hpp"

using JSON = nlohmann::json;

namespace program
{
    /**
     * ExportConfiguration of input data, which can be loaded from a JSON file.
     */
    struct InputConfiguration
    {
        InputType type;
        size_t num_elements;
        std::string data_path;
        std::pair<size_t, size_t> grid_dims;
        std::array<size_t, 3> data_dims;

        void fromJSONFile(std::string file_name);
        void toJSONFile(std::string file_name);

    private:
        const std::string KEYWORD_TYPE = "type";
        const std::string KEYWORD_GRID = "grid";
        const std::string KEYWORD_ROWS = "rows";
        const std::string KEYWORD_COLUMNS = "columns";
        const std::string KEYWORD_DATA = "data";
        const std::string KEYWORD_PATH = "path";
        const std::string KEYWORD_LENGTH = "length";
        const std::string KEYWORD_DIMENSIONS = "dimensions";
        const std::string KEYWORD_X = "x";
        const std::string KEYWORD_Y = "y";
        const std::string KEYWORD_Z = "z";
    };

    /**
     * Load the input data from a JSON file. Will throw exceptions on errors.
     * @param file_name
     */
    void InputConfiguration::fromJSONFile(std::string file_name)
    {
        std::ifstream file_stream(file_name);
        JSON parsed = JSON::parse(file_stream);

        std::string new_type = parsed[KEYWORD_TYPE];
        type = new_type == "data" ? InputType::DATA : InputType::VISUALIZATION;
        num_elements = parsed[KEYWORD_DATA][KEYWORD_LENGTH];
        data_path = parsed[KEYWORD_DATA][KEYWORD_PATH];

        size_t x = parsed[KEYWORD_GRID][KEYWORD_ROWS];
        size_t y = parsed[KEYWORD_GRID][KEYWORD_COLUMNS];
        grid_dims = { x, y };

        x = parsed[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_X];
        y = parsed[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_Y];
        size_t z = parsed[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_Z];
        data_dims = { x, y, z };
    }

    /**
     * Save the config in a JSON file. Will append .json to the filename.
     * @param file_name
     */
    void InputConfiguration::toJSONFile(std::string file_name)
    {
        JSON json;
        json[KEYWORD_TYPE] = type == InputType::DATA ? "data" : "visualization";

        json[KEYWORD_GRID][KEYWORD_ROWS] = grid_dims.first;
        json[KEYWORD_GRID][KEYWORD_COLUMNS] = grid_dims.second;

        json[KEYWORD_DATA][KEYWORD_LENGTH] = num_elements;
        json[KEYWORD_DATA][KEYWORD_PATH] = data_path;

        json[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_X] = data_dims[0];
        json[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_Y] = data_dims[1];
        json[KEYWORD_DATA][KEYWORD_DIMENSIONS][KEYWORD_Z] = data_dims[2];

        std::ofstream output_stream(file_name + ".json");
        output_stream << std::setw(4) << json << std::endl;
    }
} // program

#endif //NEW_LDG_INPUT_CONFIGURATION_HPP
