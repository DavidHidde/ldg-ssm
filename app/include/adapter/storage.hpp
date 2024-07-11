#ifndef LDG_CORE_STORAGE_HPP
#define LDG_CORE_STORAGE_HPP

#include <bzlib.h>
#include <string>
#include "app/include/ldg/model/quad_assignment_tree.hpp"
#include "data_layout.hpp"
#include "app/include/ldg/util/tree_functions.hpp"

namespace adapter
{
    constexpr uint32_t VOID_TILE_IDX = -1;

    /**
     * Read data from a .raw file into a buffer.
     * @param buf
     * @param file_name
     * @return
     */
    template<typename DataType>
    long readRawFile(std::vector<DataType> &buffer, std::string file_name)
    {
        std::ifstream file_stream(file_name.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

        if(!file_stream.is_open()) {
            std::cerr << "Could not open file: " << file_name << std::endl;
            return -1;
        }

        size_t size = file_stream.tellg();
        size_t num_elements = size / sizeof(DataType);

        file_stream.seekg(0, std::ios::beg);
        buffer.resize(num_elements);
        file_stream.read((char*)&buffer[0], num_elements * sizeof(DataType));

        return buffer.size();
    }

    /**
    * Read data from a .raw.bz2 file into a buffer.
    * @param output_buffer
    * @param file_name
    * @return
    */
    template<typename DataType>
    long readBZipFile(std::vector<DataType> &output_buffer, std::string file_name)
    {
        std::ifstream file_stream(file_name, std::ios::binary);
        if (!file_stream.is_open()) {
            std::cerr << "Could not open file: " << file_name << std::endl;
            return -1;
        }

        // Find file size
        file_stream.seekg(0, std::ios::end);
        int file_size = file_stream.tellg();
        file_stream.seekg(0, std::ios::beg);

        // Create buffers
        uint output_buf_size = output_buffer.size() * sizeof(DataType) > file_size ? output_buffer.size() * sizeof(DataType) : file_size * 2;
        output_buffer.resize(output_buf_size / sizeof(DataType));
        std::vector<char> input_buffer(file_size);

        // Read data and decompress
        file_stream.read(input_buffer.data(), file_size);
        int bz_error = BZ2_bzBuffToBuffDecompress(
            reinterpret_cast<char *>(output_buffer.data()),
            &output_buf_size,
            input_buffer.data(),
            file_size,
            0,
            0
        );

        if (bz_error != BZ_OK) {
            std::cerr << "Could not decompress file: " << file_name << std::endl;
            return -1;
        }

        output_buffer.resize(output_buf_size / sizeof(DataType));
        return output_buffer.size();
    }

    /**
     * Read data into a buffer using the appropriate function.
     *
     * @param buffer
     * @param file_name
     * @return
     */
    template<typename DataType>
    long readFileIntoBuffer(std::vector<DataType> &buffer, std::string file_name)
    {
        if (file_name.ends_with(".bz2")) {
            return readBZipFile(buffer, file_name);
        }
        if (file_name.ends_with(".raw")) {
            return readRawFile(buffer, file_name);
        }
        return -1;
    }

    /**
     * Write a compressed stream to a BZ2 file.
     *
     * @tparam DataType
     * @param input_buffer
     * @param file_name
     * @return
     */
    template<typename DataType>
    bool compressBZipFile(std::vector<DataType> &input_buffer, std::string file_name)
    {
        uint decompressed_size = input_buffer.size() * sizeof(DataType);
        uint compressed_size = std::ceil(static_cast<float>(decompressed_size) * 1.01) + 600;
        std::vector<char> output_buffer(compressed_size);

        int bz_error = BZ2_bzBuffToBuffCompress(
            output_buffer.data(),
            &compressed_size,
            reinterpret_cast<char *>(input_buffer.data()),
            decompressed_size,
            9,
            0,
            0
        );

        if (bz_error != BZ_OK) {
            std::cerr << "Could not compress file: " << file_name << std::endl;
            return false;
        }

        std::ofstream file_stream(file_name, std::ios::binary);
        if (!file_stream.is_open()) {
            std::cerr << "Could not open file: " << file_name << std::endl;
            return false;
        }
        file_stream.write(output_buffer.data(), compressed_size);

        return true;
    }

    /**
     * Save an assignment to a file.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param file_name
     */
    template<typename VectorType>
    void saveAndCompressAssignment(ldg::QuadAssignmentTree<VectorType> &quad_tree, std::string const file_name)
    {
        size_t grid_side_len = std::pow(2, std::ceil(std::log2(std::max(quad_tree.getNumRows(), quad_tree.getNumCols()))));
        std::vector assignment(grid_side_len * grid_side_len, VOID_TILE_IDX);
        copyFromRowMajorToHierarchy(quad_tree.getAssignment(), assignment, quad_tree.getNumRows(), quad_tree.getNumCols());

        // Add void tiles where the data refers to nullptrs
        for (uint32_t &value: assignment) {
            if (value != VOID_TILE_IDX && value >= quad_tree.getNumRealElements()) {
                value = VOID_TILE_IDX;
            }
        }

        compressBZipFile(assignment, file_name + ".raw.bz2");
    }

    /**
     * Read an assignment from a file.
     *
     * @param filename
     * @param num_rows
     * @param num_cols
     * @param num_actual_elements
     * @return
     */
    std::vector<size_t> readCompressedAssignment(std::string const filename, size_t num_rows, size_t num_cols, size_t num_actual_elements)
    {
        size_t max_pow2_dim = std::pow(2, std::ceil(std::log2(std::max(num_rows, num_cols))));
        std::vector<uint32_t> hierarchical_assignment(max_pow2_dim * max_pow2_dim);
        readFileIntoBuffer(hierarchical_assignment, filename);

        size_t required_assignment_capacity = ldg::determineRequiredArrayCapacity(num_rows, num_cols);
        std::vector<size_t> row_major_assignment(required_assignment_capacity);
        copyFromHierarchyToRowMajor(hierarchical_assignment, row_major_assignment, num_rows, num_cols);

        // Replace all void tiles with the end of the data array (assumed to be nullptrs) and give aggregates an assignment.
        size_t next_void_tile_placement = num_actual_elements;
        size_t num_leafs = num_rows * num_cols;
        for (size_t idx = 0; idx < required_assignment_capacity; ++idx) {
            if (row_major_assignment[idx] == VOID_TILE_IDX) {
                row_major_assignment[idx] = next_void_tile_placement++;
            }
            if (idx >= num_leafs) {
                row_major_assignment[idx] = idx;
            }
        }

        return row_major_assignment;
    }
}

#endif //LDG_CORE_STORAGE_HPP
