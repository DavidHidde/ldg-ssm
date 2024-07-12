#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "app/include/self_sorting_map/target/target_type.hpp"

#include <fstream>
#include <string>
#include <vector>

namespace program
{
    /**
     * Logger used for writing the state to a CSV file. This helps with parsing the data after the fact.
     */
    class Logger
    {
        const std::vector<std::string> header{
            "time",
            "pass",
            "height",
            "iteration",
            "distance",
            "num_exchanges",
            "max_iterations",
            "distance_threshold",
            "using_partition_swaps",
            "rows",
            "columns"
        };
        const char csv_separator = ';';

        double start_time;
        std::ofstream output_file_stream;

        // Internal counters and caches that should only change per pass
        size_t num_rows = 0;
        size_t num_cols = 0;
        size_t num_pass = 0;
        size_t max_iterations = 0;
        double distance_threshold = 0.;
        bool using_partition_swaps = true;

    public:
        explicit Logger(double start_time, std::string const &output_dir);

        void write(
            size_t height,
            size_t iteration,
            double distance,
            size_t num_exchanges
        );

        void close();

        // Flush setters
        Logger &setNumRows(size_t num_rows);
        Logger &setNumCols(size_t num_cols);
        Logger &setNumPass(size_t num_pass);
        Logger &setMaxIterations(size_t max_iterations);
        Logger &setDistanceThreshold(double distance_threshold);
        Logger &setUsingPartitionSwaps(bool using_partition_swaps);
    };

    /**
     * Start the logger, creating an output stream to a log.csv in the output directory.
     *
     * @param start_time
     * @param output_dir
     */
    inline Logger::Logger(const double start_time, std::string const &output_dir):
        start_time(start_time)
    {
        output_file_stream.open(output_dir + "log.csv");
        for (size_t idx = 0; idx < header.size(); ++idx) {
            output_file_stream << header[idx] << (idx < header.size() - 1 ? csv_separator : '\n');
        }
    }

    /**
     * Write to the log file.
     *
     * @param height
     * @param iteration
     * @param distance
     * @param num_exchanges
     */
    inline void Logger::write(const size_t height, const size_t iteration, double distance, const size_t num_exchanges)
    {
        output_file_stream <<
            omp_get_wtime() - start_time << csv_separator <<
            num_pass << csv_separator <<
            height << csv_separator <<
            iteration << csv_separator <<
            distance << csv_separator <<
            num_exchanges << csv_separator <<
            max_iterations << csv_separator <<
            distance_threshold << csv_separator <<
            using_partition_swaps << csv_separator <<
            num_rows << csv_separator <<
            num_cols << '\n';
    }

    /**
     * Close the file stream.
     */
    inline void Logger::close()
    {
        output_file_stream.close();
    }

    /**
     * @param num_rows
     * @return
     */
    inline Logger &Logger::setNumRows(size_t num_rows)
    {
        this->num_rows = num_rows;
        return *this;
    }

    /**
     * @param num_cols
     * @return
     */
    inline Logger &Logger::setNumCols(size_t num_cols)
    {
        this->num_cols = num_cols;
        return *this;
    }

    /**
     * @param num_pass
     * @return
     */
    inline Logger &Logger::setNumPass(size_t num_pass)
    {
        this->num_pass = num_pass;
        return *this;
    }

    /**
     * @param max_iterations
     * @return
     */
    inline Logger &Logger::setMaxIterations(size_t max_iterations)
    {
        this->max_iterations = max_iterations;
        return *this;
    }

    /**
     * @param distance_threshold
     * @return
     */
    inline Logger &Logger::setDistanceThreshold(double distance_threshold)
    {
        this->distance_threshold = distance_threshold;
        return *this;
    }

    /**
     * @param using_partition_swaps
     * @return
     */
    inline Logger &Logger::setUsingPartitionSwaps(bool using_partition_swaps)
    {
        this->using_partition_swaps = using_partition_swaps;
        return *this;
    }
}

#endif //LOGGER_HPP