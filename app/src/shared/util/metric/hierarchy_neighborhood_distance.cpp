#include "app/include/shared/util/metric/hierarchy_neighborhood_distance.hpp"
#include "app/include/shared/util/tree_traversal/tree_walker.hpp"
#include "app/include/shared/util/tree_traversal/row_major_iterator.hpp"

namespace shared
{
    /**
     * Compute the HND for a quad tree at a given height.
     *
     * @tparam DataType
     * @param height
     * @param distance_function
     * @param source
     * @param cache
     * @return
     */
    template<typename DataType>
    float computeHierarchyNeighborhoodDistance(
        size_t height,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source,
        QuadAssignmentTree<float> &cache
    )
    {
        float sum = 0;
        CellPosition position{ height, 0 };
        RowMajorIterator<DataType> data_iterator(height, source);
        RowMajorIterator<float> score_iterator(height, cache);

        for (; data_iterator != data_iterator.end(); ++data_iterator, ++score_iterator, ++position.index) {
            float &value = score_iterator.getValue();
            value = computeHierarchyDistanceForCell(position, distance_function, source);
        }

        score_iterator = score_iterator.begin();
        for (; score_iterator != score_iterator.end(); ++score_iterator) {
            sum += score_iterator.getValue();   // TODO: Add neighbors
        }

        return sum;
    }

    /**
     * Compute the distance to the upper hierarchy members at a certain position.
     *
     * @tparam DataType
     * @param position
     * @param distance_function
     * @param source
     * @return
     */
    template<typename DataType>
    float computeHierarchyDistanceForCell(
        CellPosition &position,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source
    )
    {
        TreeWalker<DataType> walker(position, source);
        DataType node = walker.getNode();
        float sum = 0;

        if (node != nullptr) {
            do {
                sum += distance_function(node, walker.getParent());
            } while (walker.moveUp());
        }

        return sum;
    }

    /**
     * Generate the same sized caching tree used for the HND. All values are set to empty initially.
     *
     * @tparam DataType
     * @param source
     * @return
     */
    template<typename DataType>
    QuadAssignmentTree<DataType> generateCacheTree(QuadAssignmentTree<DataType> &source)
    {
        std::vector<float> empty_scores(source.getData()->size());
        std::fill(empty_scores.begin(), empty_scores.end(), EMPTY_HND_SCORE);

        return QuadAssignmentTree<DataType>(
            std::make_shared<std::vector<float>>(empty_scores),
            source.getNumRows(),
            source.getNumCols(),
            source.getDepth()
        );
    }
}

