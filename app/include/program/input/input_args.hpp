#ifndef LDG_SSM_INPUT_ARGS_HPP
#define LDG_SSM_INPUT_ARGS_HPP

#include <iostream>
#include "cxxopts.hpp"

namespace program
{
    /**
     * Define and parse the CLI arguments.
     *
     * @param argc
     * @param argv
     * @return
     */
    cxxopts::ParseResult parseInput(int argc, const char **argv)
    {
        cxxopts::Options options("ldg_ssm");
        options.add_options()
            // IO parameters
           ("config", "Path to the config file.", cxxopts::value<std::string>())
           ("input", "Path to the previous assignment file.", cxxopts::value<std::string>())
           ("output", "Path to the output directory.", cxxopts::value<std::string>()->default_value("./"))
           // Method parameters
           ("cores", "Number of cores to use for parallel operations.", cxxopts::value<size_t>())
           ("passes", "Number of passes.", cxxopts::value<size_t>()->default_value("1"))
           ("max_iterations", "Number of maximum iterations for convergence.", cxxopts::value<size_t>()->default_value("100"))
           ("passes_per_checkpoint", "Number of passes between checkpoints. If bigger than 0, will also log the final result of a pass.", cxxopts::value<size_t>()->default_value("0"))
           ("iterations_per_checkpoint", "Number of iterations on a height between checkpoints. If bigger than 0, will also log the final result of a height.", cxxopts::value<size_t>()->default_value("0"))
           ("min_distance_change", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("0.00001"))
           ("distance_change_factor", "Distance change ratio change factor per pass.", cxxopts::value<double>()->default_value("1"))
           ("iterations_change_factor", "Maximum number of iterations change factor per pass.", cxxopts::value<double>()->default_value("1"))
           ("seed", "Randomization seed.", cxxopts::value<size_t>()->default_value(std::to_string(std::rand())))
           ("partition_swaps", "Enable partition swaps.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("randomize", "Randomize the assignment at the start.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
           ("parent_type", "Type of parent representation to use. Options are: Normalized average: 0, Minimum child: 1", cxxopts::value<size_t>()->default_value("0"))
           ("distance_function", "Distance function to use. Options are: Euclidean distance: 0, Cosine Similarity: 1", cxxopts::value<size_t>()->default_value("0"))
           ("ssm_mode", "Whether the sorting should mimic the Self-Sorting Map (SSM). This changes some parameters like the start sorting height, the target function and cell pairings.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           // Debug parameters
           ("debug", "Enable debugging (use synthetic data).", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("rows", "Number of rows of the grid.", cxxopts::value<size_t>()->default_value("128"))
           ("columns", "Number of columns of the grid.", cxxopts::value<size_t>()->default_value("128"))
           // Export parameters
           ("log_only", "Disable saving the result in any other way than a log.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("export", "Export the assignment, disparities and data if visualization data is not specified. The export can be used with the LDG-SSM interface.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
           ("visualization_config", "Path to the config for the data that visually represents the data model.", cxxopts::value<std::string>()->default_value(""))
           ("h,help", "Print usage")
       ;

        cxxopts::ParseResult result;
        try { result = options.parse(argc, argv); } catch (const cxxopts::exceptions::exception &exception) {
            std::cerr << "ldg_ssm: " << exception.what() << std::endl;
            std::cerr << "Usage: ldg_ssm [options] <input_file> ..." << std::endl;
            exit(EXIT_FAILURE);
        }
        if (result.count("help"))
        {
            std::cout << options.help() << std::endl;
            exit(EXIT_SUCCESS);
        }

        return result;
    }
}

#endif //LDG_SSM_INPUT_ARGS_HPP
