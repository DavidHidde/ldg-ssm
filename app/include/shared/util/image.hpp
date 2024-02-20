#ifndef LDG_CORE_IMAGE_HPP
#define LDG_CORE_IMAGE_HPP

#include <string>
#include "CImg.h"
#include "app/include/shared/model/quad_assignment_tree.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"

namespace shared
{
    /**
     * Save an RGB image of the quad tree. We assume the data vector to at least be of length 3.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param filename
     * @param height
     */
    template<typename VectorType>
    void saveImage(QuadAssignmentTree<VectorType> &quad_tree, std::string const filename, size_t const height)
    {
        using namespace cimg_library;
        auto dimensions = quad_tree.getBounds(CellPosition{ height, 0 }).second;    // num_rows, num_cols
        CImg<float> img(dimensions.second, dimensions.first, 1, 3);
        for (RowMajorIterator<VectorType> it(height, quad_tree); it != it.end(); ++it) {
            auto position = it.getPosition();
            size_t x = position.index % dimensions.second;
            size_t y = position.index / dimensions.second;
            auto value = *it.getValue();
            img(x, y, 0, 0) = value.x;
            img(x, y, 0, 1) = value.y;
            img(x, y, 0, 2) = value.z;
        }
        img.save_png((filename + ".png").c_str());
    }

    /**
     * Save an RGB image of every height level of the quad tree. Every height gets a -(h<height>) appended.
     *
     * @tparam VectorType
     * @param quad_tree
     * @param filename
     * @param height
     */
    template<typename VectorType>
    void saveQuadTreeImages(QuadAssignmentTree<VectorType> &quad_tree, std::string const base_filename)
    {
        for (size_t height = 0; height < quad_tree.getDepth(); ++height)
            saveImage(quad_tree, base_filename + "-(h" + std::to_string(height) + ")", height);
    }
}

#endif //LDG_CORE_IMAGE_HPP
