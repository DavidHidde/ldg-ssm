#ifndef NEW_LDG_EXPORT_SETTINGS_HPP
#define NEW_LDG_EXPORT_SETTINGS_HPP

#include <string>
#include "export_configuration.hpp"

namespace program
{
    /**
     *  Settings needed for export.
     */
    struct ExportSettings
    {
        std::string output_dir;
        std::string file_name;

        std::string visualization_config_path;

        bool log_only = false; // Don't export.
        bool debug = false; // Debug mode prints images across all heights for RGB configurations.
        bool export_data = false;
        bool export_disparity = false;
    };
} // program

#endif //NEW_LDG_EXPORT_SETTINGS_HPP
