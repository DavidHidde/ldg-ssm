#ifndef LDG_SSM_INPUT_ARGS_HPP
#define LDG_SSM_INPUT_ARGS_HPP

#include <iostream>
#include "app/cmake-build-debug/_deps/cxxopts-src/include/cxxopts.hpp"

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
           ("iterations_per_checkpoint", "Number of iterations between checkpoints.", cxxopts::value<size_t>()->default_value("0"))
           ("min_distance_change", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("0.00001"))
           ("distance_change_factor", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("1"))
           ("iterations_change_factor", "Minimum distance change for convergence.", cxxopts::value<double>()->default_value("1"))
           ("seed", "Randomization seed.", cxxopts::value<size_t>()->default_value(std::to_string(std::rand())))
           ("partition_swaps", "Enable partition swaps.", cxxopts::value<bool>()->default_value("true")->implicit_value("false"))
           ("randomize", "Randomize the assignment at the start.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
           ("combine_targets", "Combine targets into a single target. If false, one target will be used per pass, with the last target repeating until the end.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("distance_function", "Distance function to use. Options are: Euclidean distance: 0, Cosine Similarity: 1", cxxopts::value<size_t>()->default_value("0"))
           ("targets", "Targets of the SSM. Options are: Aggregate Hierarchy: 0, Highest Parent: 1, Aggregate Hierarchy (4 connected): 2, Highest Parent (4 connected): 3, Partition Neighbourhood: 4, Cell Neighbourhood: 5", cxxopts::value<std::vector<size_t>>()->default_value("4"))
           // Debug parameters
           ("debug", "Enable debugging (use synthetic data).", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("rows", "Number of rows of the grid.", cxxopts::value<size_t>()->default_value("128"))
           ("columns", "Number of columns of the grid.", cxxopts::value<size_t>()->default_value("128"))
           // Export parameters
           ("log_only", "Disable saving the result in any other way than a log.", cxxopts::value<bool>()->default_value("false")->implicit_value("true"))
           ("export", "Export the assignment, disparities and data if visualization data is not specified. The export can be used with the LDG-SSM interface.", cxxopts::value<bool>()->default_value("true")->implicit_value("true"))
           ("visualization_config", "Path to the config for the data that visually represents the data model.", cxxopts::value<std::string>()->default_value(""))
       ;

        cxxopts::ParseResult result;
        try { result = options.parse(argc, argv); } catch (const cxxopts::exceptions::exception &exception) {
            std::cerr << "ldg_ssm: " << exception.what() << std::endl;
            std::cerr << "Usage: ldg_ssm [options] <input_file> ..." << std::endl;
            exit(EXIT_FAILURE);
        }

        return result;
    }
}

#endif //LDG_SSM_INPUT_ARGS_HPP
