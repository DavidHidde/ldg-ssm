#include "app/include/shared/util/metric/hierarchy_neighborhood_distance.hpp"

namespace shared
{
    template<typename DataType>
    float computeHierarchyNeighborhoodDistance(
        size_t height,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source,
        QuadAssignmentTree<float> &cache
    )
    {
        return 0;
    }

    /**
     * Compute the HND for a single cell in the quad tree using a predefined caching function.
     * Uses a caching tree to cache intermediate values.
     *
     * @tparam DataType
     * @param distance_function
     * @param position
     * @param source
     * @param cache
     * @return
     */
    template<typename DataType>
    float computeHierarchyNeighborhoodDistanceForCell(
        CellPosition &position,
        std::function<float(DataType, DataType)> distance_function,
        QuadAssignmentTree<DataType> &source,
        QuadAssignmentTree<float> &cache
    )
    {


        return 0;
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

